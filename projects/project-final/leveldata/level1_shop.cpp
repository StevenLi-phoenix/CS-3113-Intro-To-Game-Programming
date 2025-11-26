#include "level1.h"
#include "level1_consts.h"
#include "../lib/ResourceManager.h"

using namespace level1_consts;

namespace
{
    constexpr float SHOP_PANEL_WIDTH = 860.0f;
    constexpr float SHOP_PANEL_HEIGHT = 360.0f;
    constexpr float SHOP_PANEL_MARGIN_TOP = 30.0f;
    constexpr float SHOP_BUTTON_WIDTH = 240.0f;
    constexpr float SHOP_BUTTON_HEIGHT = 72.0f;
    constexpr float SHOP_BUTTON_ROW_SPACING = 110.0f;
    constexpr float SHOP_BUTTON_COL_SPACING = 260.0f;
    constexpr float SHOP_BUTTON_TOP_OFFSET = 160.0f;
    constexpr float SHOP_ICON_SIZE = 56.0f;

    Rectangle shopPanelRect()
    {
        return {
            (c::SCREEN_WIDTH - SHOP_PANEL_WIDTH) * 0.5f,
            SHOP_PANEL_MARGIN_TOP + (c::SCREEN_HEIGHT - SHOP_PANEL_HEIGHT) * 0.5f - SHOP_PANEL_MARGIN_TOP * 0.5f,
            SHOP_PANEL_WIDTH,
            SHOP_PANEL_HEIGHT
        };
    }
}

void Level1::ensureShopUI()
{
    if (!mShopButtons.empty())
    {
        return;
    }

    auto makeButton = []() -> std::unique_ptr<Button>
    {
        auto btn = std::make_unique<Button>();
        btn->setScale({SHOP_BUTTON_WIDTH, SHOP_BUTTON_HEIGHT});
        btn->setBackgroundColor(Fade(DARKBLUE, 0.78f));
        btn->setBorderColor(Fade(RAYWHITE, 0.8f));
        btn->setTextColor(RAYWHITE);
        btn->setFontSize(22);
        return btn;
    };

    mShopButtons.push_back(makeButton());
    mShopButtons.push_back(makeButton());
    mShopButtons.push_back(makeButton());
    mShopButtons.push_back(makeButton());
    updateShopButtonsLayout();
}

void Level1::updateShopButtonsLayout()
{
    if (mShopButtons.size() < 4)
    {
        return;
    }
    const Rectangle panel = shopPanelRect();
    const float centerX = panel.x + panel.width * 0.5f;
    const float topY = panel.y + SHOP_BUTTON_TOP_OFFSET;

    mShopButtons[0]->setPosition({centerX - SHOP_BUTTON_COL_SPACING, topY});
    mShopButtons[1]->setPosition({centerX + SHOP_BUTTON_COL_SPACING, topY});
    mShopButtons[2]->setPosition({centerX - SHOP_BUTTON_COL_SPACING * 0.5f, topY + SHOP_BUTTON_ROW_SPACING});
    mShopButtons[3]->setPosition({centerX + SHOP_BUTTON_COL_SPACING * 0.5f, topY + SHOP_BUTTON_ROW_SPACING});
}

void Level1::handleShopClose()
{
    mShopOpen = false;
    Button::updateGlobalCursor();
}

void Level1::updateShop(float deltaTime)
{
    (void)deltaTime;
    const bool nearTable = isPlayerNearTable(SHOP_INTERACT_RADIUS);
    if (!nearTable)
    {
        if (mShopOpen)
        {
            handleShopClose();
        }
        mShopSuppressed = false;
        return;
    }

    if (!mBossSpawned)
    {
        spawnBoss();
    }

    if (mShopSuppressed)
    {
        return;
    }

    ensureShopUI();
    updateShopButtonsLayout();

    mShopOpen = true;

    if (IsKeyPressed(KEY_Q))
    {
        handleShopClose();
        mShopSuppressed = true;
        return;
    }

    auto purchaseSword = [&]()
    {
        if (mGoldCount < SWORD_COST)
        {
            return;
        }
        mGoldCount -= SWORD_COST;
        ++mSwordUpgradeCount;
        mMeleeDamage = combat::MELEE_DAMAGE *
                       (1.0f + static_cast<float>(mSwordUpgradeCount) * SWORD_DAMAGE_BONUS);
        syncGoldSlot();
        syncWeaponSlot();
        playPurchaseSFX(true);
        LOG_INFO(TextFormat("Purchased Sword level %d, melee damage=%.1f",
                            mSwordUpgradeCount,
                            mMeleeDamage));
    };

    auto purchaseShuriken = [&]()
    {
        if (mGoldCount < SHURIKEN_COST)
        {
            return;
        }
        mGoldCount -= SHURIKEN_COST;
        ++mShurikenUpgradeCount;
        mRecoverableThrows = true;
        mBranchCapacity = std::max(mBranchCapacity, 999);
        mBranchDamage = branch::PROJECTILE_DAMAGE *
                        (1.0f + static_cast<float>(mShurikenUpgradeCount) * SHURIKEN_DAMAGE_BONUS);
        mBranchInventory = std::clamp(mBranchInventory + 1, 0, mBranchCapacity);
        syncGoldSlot();
        syncBranchSlot();
        playPurchaseSFX(true);
        LOG_INFO(TextFormat("Purchased Shuriken level %d, throw damage=%.1f",
                            mShurikenUpgradeCount,
                            mBranchDamage));
    };

    auto purchasePotion = [&]()
    {
        if (mGoldCount < POTION_COST)
        {
            return;
        }
        if (mPotionCount >= mPotionCapacity)
        {
            return;
        }
        mGoldCount -= POTION_COST;
        addPotions(1);
        syncGoldSlot();
        playPurchaseSFX(true);
        LOG_INFO(TextFormat("Purchased potion (%d/%d)", mPotionCount, mPotionCapacity));
    };

    if (IsKeyPressed(KEY_ONE))
    {
        purchaseSword();
    }
    if (IsKeyPressed(KEY_TWO))
    {
        purchaseShuriken();
    }
    if (IsKeyPressed(KEY_THREE))
    {
        purchasePotion();
    }

    if (mShopButtons.size() >= 4)
    {
        ResourceManager &rm = ResourceManager::instance();
        Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
        Rectangle swordRect = rm.getSpriteRect(tags::AXE);
        Rectangle shurikenRect = rm.getSpriteRect(tags::SHURIKEN);
        Rectangle potionRect = rm.getSpriteRect(tags::POTION);

        const std::string swordLabel = TextFormat("%dG", SWORD_COST);
        const std::string shurikenLabel = TextFormat("%dG", SHURIKEN_COST);
        const std::string potionLabel = TextFormat("%dG", POTION_COST);
        mShopButtons[0]->setText(swordLabel);
        mShopButtons[1]->setText(shurikenLabel);
        mShopButtons[2]->setText(potionLabel);
        mShopButtons[3]->setText("Close (Q)");

        const Color affordSword = mGoldCount >= SWORD_COST ? Fade(DARKGREEN, 0.82f) : Fade(DARKGRAY, 0.82f);
        const Color affordShuriken = mGoldCount >= SHURIKEN_COST ? Fade(DARKGREEN, 0.82f) : Fade(DARKGRAY, 0.82f);
        const Color affordPotion = (mGoldCount >= POTION_COST && mPotionCount < mPotionCapacity)
            ? Fade(DARKGREEN, 0.82f)
            : Fade(DARKGRAY, 0.82f);
        mShopButtons[0]->setBackgroundColor(affordSword);
        mShopButtons[1]->setBackgroundColor(affordShuriken);
        mShopButtons[2]->setBackgroundColor(affordPotion);
        mShopButtons[3]->setBackgroundColor(Fade(MAROON, 0.78f));

        if (atlas && swordRect.width > 0.0f && swordRect.height > 0.0f)
        {
            mShopButtons[0]->setIcon(atlas, swordRect, {SHOP_ICON_SIZE, SHOP_ICON_SIZE});
        }
        if (atlas && shurikenRect.width > 0.0f && shurikenRect.height > 0.0f)
        {
            mShopButtons[1]->setIcon(atlas, shurikenRect, {SHOP_ICON_SIZE, SHOP_ICON_SIZE});
        }
        if (atlas && potionRect.width > 0.0f && potionRect.height > 0.0f)
        {
            mShopButtons[2]->setIcon(atlas, potionRect, {SHOP_ICON_SIZE, SHOP_ICON_SIZE});
        }
        mShopButtons[3]->setIcon(nullptr, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f});

        mShopButtons[0]->setOnClick(purchaseSword);
        mShopButtons[1]->setOnClick(purchaseShuriken);
        mShopButtons[2]->setOnClick(purchasePotion);
        mShopButtons[3]->setOnClick([&]() { handleShopClose(); mShopSuppressed = true; });

        for (auto &btn : mShopButtons)
        {
            if (btn)
            {
                btn->update(deltaTime);
            }
        }
    }
}

void Level1::drawShopOverlay() const
{
    if (!mShopOpen)
    {
        return;
    }

    DrawRectangle(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, Fade(BLACK, 0.55f));

    Rectangle panel = shopPanelRect();
    DrawRectangleRounded(panel, 0.2f, 6, Fade(BLACK, 0.6f));
    DrawRectangleLinesEx(panel, 2.0f, Fade(WHITE, 0.5f));

    const char *title = "Map Table Shop (Paused)";
    const int titleSize = 22;
    const int titleWidth = MeasureText(title, titleSize);
    DrawText(title,
             static_cast<int>(panel.x + (panel.width - titleWidth) * 0.5f),
             static_cast<int>(panel.y + 10.0f),
             titleSize,
             RAYWHITE);

    const std::string swordLine = TextFormat("Sword bonus: +%d%%   Melee: %.1f",
                                             mSwordUpgradeCount * static_cast<int>(SWORD_DAMAGE_BONUS * 100.0f),
                                             mMeleeDamage);
    const std::string shurikenLine = TextFormat("Shuriken bonus: +%d%%   Throw: %.1f   Reclaimable",
                                                mShurikenUpgradeCount * static_cast<int>(SHURIKEN_DAMAGE_BONUS * 100.0f),
                                                mBranchDamage);
    const std::string potionLine = TextFormat("Potions: %d / %d   Heals +%.1f HP",
                                              mPotionCount,
                                              mPotionCapacity,
                                              POTION_HEAL_AMOUNT);
    DrawText(swordLine.c_str(),
             static_cast<int>(panel.x + 18.0f),
             static_cast<int>(panel.y + 60.0f),
             20,
             LIGHTGRAY);
    DrawText(shurikenLine.c_str(),
             static_cast<int>(panel.x + 18.0f),
             static_cast<int>(panel.y + 90.0f),
             20,
             LIGHTGRAY);
    DrawText(potionLine.c_str(),
             static_cast<int>(panel.x + 18.0f),
             static_cast<int>(panel.y + 120.0f),
             20,
             LIGHTGRAY);

    const std::string goldLine = TextFormat("Gold: %d", mGoldCount);
    DrawText(goldLine.c_str(),
             static_cast<int>(panel.x + panel.width - 200.0f),
             static_cast<int>(panel.y + 62.0f),
             20,
             GOLD);

    const char *hint = "Game frozen: click icons or press 1/2/3 to buy (stacks), press Q or Close to exit, reclaim throws by walking over them";
    DrawText(hint,
             static_cast<int>(panel.x + 18.0f),
             static_cast<int>(panel.y + panel.height - 38.0f),
             17,
             Fade(RAYWHITE, 0.92f));

    for (const auto &btn : mShopButtons)
    {
        if (btn)
        {
            btn->render();
        }
    }
}
