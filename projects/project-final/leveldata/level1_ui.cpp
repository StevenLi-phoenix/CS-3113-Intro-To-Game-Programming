#include "level1.h"
#include "level1_consts.h"
#include "../lib/ResourceManager.h"

using namespace level1_consts;

void Level1::drawPlayerHUD() const
{
    if (!mPlayer)
    {
        return;
    }

    const float maxHealth = std::max(mPlayer->getMaxHealth(), 0.001f);
    const float healthRatio = std::clamp(mPlayer->getHealth() / maxHealth, 0.0f, 1.0f);
    const Vector2 playerScreenPos = GetWorldToScreen2D(mPlayer->getPosition(), mCamera);

    const Vector2 playerScale = mPlayer->getScale();
    const float barWidth = std::max(playerScale.x * 0.9f, 60.0f);
    const float barHeight = 8.0f;
    const float verticalOffset = -playerScale.y * 0.65f;

    Rectangle barBackground = {
        playerScreenPos.x - barWidth * 0.5f,
        playerScreenPos.y + verticalOffset - barHeight,
        barWidth,
        barHeight
    };

    Rectangle barFill = barBackground;
    barFill.width = barBackground.width * healthRatio;

    DrawRectangleRounded(barBackground, 0.4f, 8, Fade(BLACK, 0.55f));
    DrawRectangleRec(barFill, RED);
    DrawRectangleLinesEx(barBackground, 1.0f, Fade(WHITE, 0.85f));
}

void Level1::drawInventoryOverlay()
{
    if (mInventoryBar)
    {
        mInventoryBar->render();
    }
}

void Level1::updateInventoryUI(float deltaTime)
{
    if (mInventoryBar)
    {
        mInventoryBar->update(deltaTime);
    }
    updateToolHint(deltaTime);
}

void Level1::updateTutorialOverlay(float deltaTime)
{
    if (!mTutorialOverlayVisible)
    {
        if (IsKeyPressed(TUTORIAL_REOPEN_KEY))
        {
            mTutorialOverlayVisible = true;
            mTutorialOverlayDismissed = false;
            mTutorialOverlayDisplayTimer = 0.0f;
            mTutorialOverlayFadeTimer = 0.0f;
        }
        if (mTutorialReopenHintTimer > 0.0f)
        {
            mTutorialReopenHintTimer = std::max(0.0f, mTutorialReopenHintTimer - deltaTime);
        }
        return;
    }

    mTutorialOverlayDisplayTimer += deltaTime;

    if (!mTutorialOverlayDismissed)
    {
        const bool interacted = tutorialInputDetected();
        if (interacted || mTutorialOverlayDisplayTimer >= tutorial::AUTO_HIDE_SECONDS)
        {
            mTutorialOverlayDismissed = true;
            mTutorialOverlayFadeTimer = 0.0f;
            mTutorialReopenHintTimer = TUTORIAL_REOPEN_HINT_SECONDS;
        }
        return;
    }

    mTutorialOverlayFadeTimer += deltaTime;
    if (mTutorialOverlayFadeTimer >= tutorial::FADE_SECONDS)
    {
        mTutorialOverlayVisible = false;
    }
}

void Level1::drawTutorialOverlay() const
{
    if (!mTutorialOverlayVisible || isPaused())
    {
        if (mTutorialReopenHintTimer > 0.0f && !isPaused())
        {
            const float t = std::clamp(mTutorialReopenHintTimer / TUTORIAL_REOPEN_HINT_SECONDS, 0.0f, 1.0f);
            const float alphaHint = std::clamp(t, 0.0f, 1.0f);
            const char *hint = "Press F2 to reopen the tutorial tips";
            const int fontSize = 18;
            const int width = MeasureText(hint, fontSize);
            DrawText(hint,
                     (c::SCREEN_WIDTH - width) / 2,
                     26,
                     fontSize,
                     Fade(RAYWHITE, alphaHint));
        }
        return;
    }

    const float alpha = tutorialOverlayAlpha();
    if (alpha <= 0.0f)
    {
        return;
    }

    const float headerHeight = 70.0f;
    const float lineSpacing = 28.0f;
    const float footerHeight = 70.0f;
    const float panelHeight = headerHeight +
                              static_cast<float>(tutorial::LINE_COUNT) * lineSpacing +
                              footerHeight;

    DrawRectangle(0,
                  0,
                  c::SCREEN_WIDTH,
                  static_cast<int>(panelHeight),
                  Fade(BLACK, 0.55f * alpha));

    const int titleFontSize = 32;
    const int titleWidth = MeasureText(tutorial::TITLE, titleFontSize);
    DrawText(tutorial::TITLE,
             (c::SCREEN_WIDTH - titleWidth) / 2,
             18,
             titleFontSize,
             Fade(RAYWHITE, alpha));

    int lineY = 70;
    const int lineFontSize = 22;
    for (size_t i = 0; i < tutorial::LINE_COUNT; ++i)
    {
        const char *line = tutorial::LINES[i];
        const int lineWidth = MeasureText(line, lineFontSize);
        DrawText(line,
                 (c::SCREEN_WIDTH - lineWidth) / 2,
                 lineY,
                 lineFontSize,
                 Fade(LIGHTGRAY, alpha));
        lineY += static_cast<int>(lineSpacing);
    }

    const char *dismissHint = "Move, click, or wait a moment to hide this hint";
    const char *reopenHint = "Press F2 anytime to reopen these tips";
    const int hintFontSize = 18;
    const int hintWidth = MeasureText(dismissHint, hintFontSize);
    DrawText(dismissHint,
             (c::SCREEN_WIDTH - hintWidth) / 2,
             lineY,
             hintFontSize,
             Fade(GRAY, alpha));
    const int reopenWidth = MeasureText(reopenHint, hintFontSize);
    DrawText(reopenHint,
             (c::SCREEN_WIDTH - reopenWidth) / 2,
             lineY + 24,
             hintFontSize,
             Fade(LIGHTGRAY, alpha));
}

bool Level1::tutorialInputDetected() const
{
    for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; ++key)
    {
        if (IsKeyPressed(static_cast<KeyboardKey>(key)))
        {
            return true;
        }
    }

    constexpr MouseButton mouseButtons[] = {
        MOUSE_BUTTON_LEFT,
        MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_MIDDLE
    };
    for (MouseButton button : mouseButtons)
    {
        if (IsMouseButtonPressed(button))
        {
            return true;
        }
    }

    for (int pad = 0; pad < tutorial::MAX_GAMEPADS; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }

        for (int button = GAMEPAD_BUTTON_UNKNOWN + 1;
             button <= GAMEPAD_BUTTON_RIGHT_THUMB;
             ++button)
        {
            if (IsGamepadButtonPressed(pad, static_cast<GamepadButton>(button)))
            {
                return true;
            }
        }

        const int axisCount = GetGamepadAxisCount(pad);
        for (int axis = 0; axis < axisCount; ++axis)
        {
            if (std::fabs(GetGamepadAxisMovement(pad, axis)) > 0.35f)
            {
                return true;
            }
        }
    }

    return false;
}

float Level1::tutorialOverlayAlpha() const
{
    if (!mTutorialOverlayVisible)
    {
        return 0.0f;
    }
    if (!mTutorialOverlayDismissed)
    {
        return 1.0f;
    }
    const float remaining = 1.0f - (mTutorialOverlayFadeTimer / tutorial::FADE_SECONDS);
    return std::clamp(remaining, 0.0f, 1.0f);
}

void Level1::drawCompassIndicator()
{
    if (!mCompassUI)
    {
        return;
    }
    const bool shouldShow = isCompassSelected() && !mQuestComplete;
    mCompassUI->setIsActive(shouldShow);
    if (shouldShow)
    {
        mCompassUI->render();
    }
}

static std::string toolHintForSlot(size_t index,
                                   size_t axeSlot,
                                   size_t compassSlot,
                                   size_t branchSlot,
                                   size_t potionSlot)
{
    if (index == axeSlot)
    {
        return "Sword: press Z to slash nearby foes or chop trees.";
    }
    if (index == compassSlot)
    {
        return "Compass: keep selected to see the map-table direction marker.";
    }
    if (index == branchSlot)
    {
        return "Branches: left-click or press Z to throw toward the cursor.";
    }
    if (index == potionSlot)
    {
        return "Potions: press Z while selected to heal if injured.";
    }
    return "";
}

void Level1::updateToolHint(float deltaTime)
{
    if (!mInventory)
    {
        return;
    }

    const int selected = static_cast<int>(mInventory->getSelectedIndex());
    if (selected != mLastSelectedSlot)
    {
        mLastSelectedSlot = selected;
        mToolHintText = toolHintForSlot(static_cast<size_t>(selected),
                                        mAxeSlotIndex,
                                        mCompassSlotIndex,
                                        mBranchSlotIndex,
                                        mPotionSlotIndex);
        mToolHintTimer = mToolHintText.empty() ? 0.0f : 3.5f;
    }

    if (mToolHintTimer > 0.0f)
    {
        mToolHintTimer = std::max(0.0f, mToolHintTimer - deltaTime);
    }
}

void Level1::drawToolHint() const
{
    if (mToolHintTimer <= 0.0f || mToolHintText.empty())
    {
        return;
    }

    const float t = mToolHintTimer / 3.5f;
    const float alpha = std::clamp(t, 0.0f, 1.0f);

    const int fontSize = 20;
    const int textWidth = MeasureText(mToolHintText.c_str(), fontSize);
    Rectangle panel = {
        (c::SCREEN_WIDTH - textWidth) * 0.5f - 16.0f,
        static_cast<float>(c::SCREEN_HEIGHT) - 130.0f,
        static_cast<float>(textWidth) + 32.0f,
        34.0f
    };
    DrawRectangleRounded(panel, 0.25f, 6, Fade(BLACK, 0.55f * alpha));
    DrawRectangleLinesEx(panel, 2.0f, Fade(RAYWHITE, 0.5f * alpha));
    DrawText(mToolHintText.c_str(),
             static_cast<int>(panel.x + 16.0f),
             static_cast<int>(panel.y + 8.0f),
             fontSize,
             Fade(RAYWHITE, alpha));
}

void Level1::drawMapTableUI() const
{
    if (!mTable || !mPlayer || isPaused())
    {
        return;
    }

    const float hintRadius = level1_consts::SHOP_INTERACT_RADIUS * 1.1f;
    const bool nearTable = isPlayerNearTable(hintRadius);
    if (!nearTable && !mShopOpen)
    {
        return;
    }

    Vector2 tableScreen = GetWorldToScreen2D(mTable->getPosition(), mCamera);
    const char *label = mShopOpen ? "Map Table Shop (Q to close)" : "Map Table - shop will open here";
    const int fontSize = 18;
    const int textWidth = MeasureText(label, fontSize);
    Rectangle bg = {
        tableScreen.x - textWidth * 0.6f,
        tableScreen.y - 56.0f,
        static_cast<float>(textWidth) * 1.2f + 16.0f,
        32.0f
    };
    DrawRectangleRounded(bg, 0.25f, 6, Fade(BLACK, 0.55f));
    DrawRectangleLinesEx(bg, 2.0f, Fade(WHITE, 0.5f));
    DrawText(label,
             static_cast<int>(tableScreen.x - textWidth * 0.5f),
             static_cast<int>(bg.y + 6.0f),
             fontSize,
             Fade(RAYWHITE, 0.95f));
}
