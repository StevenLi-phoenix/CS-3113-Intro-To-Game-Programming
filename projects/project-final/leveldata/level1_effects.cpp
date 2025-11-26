#include "level1.h"
#include "level1_consts.h"
#include "../lib/ResourceManager.h"
#include "../lib/Music.h"

using namespace level1_consts;

namespace
{
    constexpr const char *SFX_MELEE_SWING[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/07_human_atk_sword_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/07_human_atk_sword_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/07_human_atk_sword_3.wav"
    };
    constexpr size_t SFX_MELEE_SWING_COUNT = sizeof(SFX_MELEE_SWING) / sizeof(SFX_MELEE_SWING[0]);

    constexpr const char *SFX_MELEE_HIT[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/26_sword_hit_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/26_sword_hit_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/26_sword_hit_3.wav"
    };
    constexpr size_t SFX_MELEE_HIT_COUNT = sizeof(SFX_MELEE_HIT) / sizeof(SFX_MELEE_HIT[0]);

    constexpr const char *SFX_THROW[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/27_sword_miss_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/27_sword_miss_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/27_sword_miss_3.wav"
    };
    constexpr size_t SFX_THROW_COUNT = sizeof(SFX_THROW) / sizeof(SFX_THROW[0]);

    constexpr const char *SFX_GOLD_PICKUP[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/04_sack_open_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/04_sack_open_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/04_sack_open_3.wav"
    };
    constexpr size_t SFX_GOLD_PICKUP_COUNT = sizeof(SFX_GOLD_PICKUP) / sizeof(SFX_GOLD_PICKUP[0]);

    constexpr const char *SFX_POTION[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/08_human_charge_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/08_human_charge_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/10_human_special_atk_1.wav"
    };
    constexpr size_t SFX_POTION_COUNT = sizeof(SFX_POTION) / sizeof(SFX_POTION[0]);

    constexpr const char *SFX_PURCHASE_SUCCESS[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/02_chest_close_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/02_chest_close_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/02_chest_close_3.wav"
    };
    constexpr size_t SFX_PURCHASE_SUCCESS_COUNT = sizeof(SFX_PURCHASE_SUCCESS) / sizeof(SFX_PURCHASE_SUCCESS[0]);

    constexpr const char *SFX_ENEMY_DEATH[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/24_orc_death_spin.wav"
    };
    constexpr size_t SFX_ENEMY_DEATH_COUNT = sizeof(SFX_ENEMY_DEATH) / sizeof(SFX_ENEMY_DEATH[0]);

    void playRandomSFX(const char* const* options, size_t count)
    {
        if (!options || count == 0)
        {
            return;
        }
        const int index = GetRandomValue(0, static_cast<int>(count) - 1);
        AudioManager::playSFX(options[index]);
    }
}

void Level1::ensureHurtShader()
{
    if (mHurtShaderReady)
    {
        return;
    }
    mHurtShaderReady = mHurtShader.load("assets/shaders/hurt_overlay.vs",
                                        "assets/shaders/hurt_overlay.fs");
    if (!mHurtShaderReady)
    {
        LOG_WARNING("Hurt shader failed to load; falling back to simple overlay.");
    }
}

void Level1::updateHurtOverlay(float deltaTime)
{
    if (!mPlayer)
    {
        mHurtOverlayTimer = 0.0f;
        return;
    }

    const float currentHealth = mPlayer->getHealth();
    if (currentHealth < mLastPlayerHealth - 0.01f)
    {
        mHurtOverlayTimer = HURT_FLASH_DURATION;
    }
    mLastPlayerHealth = currentHealth;

    if (mHurtOverlayTimer > 0.0f)
    {
        mHurtOverlayTimer = std::max(0.0f, mHurtOverlayTimer - deltaTime);
    }
}

void Level1::drawHurtOverlay()
{
    if (mHurtOverlayTimer <= 0.0f)
    {
        return;
    }

    const float normalized = std::clamp(mHurtOverlayTimer / HURT_FLASH_DURATION, 0.0f, 1.0f);
    const float pulse = 0.6f + 0.4f * sinf(static_cast<float>(GetTime()) * HURT_FLASH_PULSE);
    const float intensity = std::clamp(normalized * pulse, 0.0f, 1.0f);

    if (mHurtShaderReady)
    {
        mHurtShader.setFloat("u_intensity", intensity);
        mHurtShader.setFloat("u_time", static_cast<float>(GetTime()));
        mHurtShader.begin();
        DrawRectangle(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, Fade(RED, HURT_FLASH_ALPHA));
        mHurtShader.end();
    }
    else
    {
        DrawRectangle(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, Fade(RED, intensity * HURT_FLASH_ALPHA));
    }
}

void Level1::updateMeleeEffects(float deltaTime)
{
    auto it = mMeleeEffects.begin();
    while (it != mMeleeEffects.end())
    {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration)
        {
            it = mMeleeEffects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::spawnMeleeEffect()
{
    if (!mPlayer)
    {
        return;
    }
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    Rectangle sheet = rm.getSpriteRect(tags::WINDCHARGE);
    if (!atlas || sheet.width <= 0.0f || sheet.height <= 0.0f)
    {
        if (isDebugMode())
        {
            LOG_WARNING("Windcharge sprite missing; melee effect skipped.");
        }
        return;
    }

    MeleeEffect fx;
    fx.position = mPlayer->getPosition();
    fx.elapsed = 0.0f;
    fx.duration = MELEE_FX_DURATION;
    fx.frameCount = 4;
    fx.facesLeft = mPlayer->getIsHorizontalFlipped();
    const float frameWidth = sheet.width / static_cast<float>(std::max(fx.frameCount, 1));
    const float targetWidth = mPlayer->getScale().x * 1.2f;
    fx.scale = (frameWidth > 0.0f) ? std::max(targetWidth / frameWidth, 0.1f) : 1.0f;

    mMeleeEffects.push_back(fx);
}

void Level1::drawMeleeEffects()
{
    if (mMeleeEffects.empty())
    {
        return;
    }
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    Rectangle sheet = rm.getSpriteRect(tags::WINDCHARGE);
    if (!atlas || sheet.width <= 0.0f || sheet.height <= 0.0f)
    {
        return;
    }

    for (const MeleeEffect &fx : mMeleeEffects)
    {
        const int frames = std::max(fx.frameCount, 1);
        const float frameWidth = sheet.width / static_cast<float>(frames);
        const float progress = std::clamp(fx.elapsed / std::max(fx.duration, 0.0001f), 0.0f, 1.0f);
        const int frameIndex = std::min(frames - 1, static_cast<int>(progress * frames));
        Rectangle src = {
            sheet.x + frameWidth * static_cast<float>(frameIndex),
            sheet.y,
            frameWidth,
            sheet.height
        };
        if (fx.facesLeft)
        {
            src.width = -frameWidth;
        }

        const float alpha = std::clamp(1.0f - progress, 0.0f, 1.0f);
        const float destWidth = std::abs(frameWidth) * fx.scale;
        const float destHeight = sheet.height * fx.scale;
        const float forward = fx.facesLeft ? -MELEE_FX_FORWARD_OFFSET : MELEE_FX_FORWARD_OFFSET;

        Rectangle dest = {
            fx.position.x + forward - destWidth * 0.5f,
            fx.position.y - destHeight * 0.45f,
            destWidth,
            destHeight
        };

        DrawTexturePro(*atlas, src, dest, {0.0f, 0.0f}, 0.0f, Fade(WHITE, alpha));
    }
}

void Level1::updateSpreadProjectiles(float deltaTime)
{
    auto it = mSpreadProjectiles.begin();
    while (it != mSpreadProjectiles.end())
    {
        it->frameTimer += deltaTime;
        if (it->frameTimer >= level1_consts::SPREAD_BALL_FRAME_TIME)
        {
            it->frame = (it->frame + 1) % std::max(level1_consts::SPREAD_BALL_FRAMES, 1);
            it->frameTimer = 0.0f;
        }

        it->lifetime -= deltaTime;
        it->position.x += it->velocity.x * it->speed * deltaTime;
        it->position.y += it->velocity.y * it->speed * deltaTime;
        if (it->lifetime <= 0.0f)
        {
            it = mSpreadProjectiles.erase(it);
            continue;
        }
        if (mPlayer && mPlayer->getIsActive())
        {
            Vector2 diff = {
                it->position.x - mPlayer->getPosition().x,
                it->position.y - mPlayer->getPosition().y
            };
            const float distSq = diff.x * diff.x + diff.y * diff.y;
            const float playerRadius = std::max(mPlayer->getColliderDimensions().x,
                                                mPlayer->getColliderDimensions().y) * 0.5f;
            const float collideRadius = it->radius + playerRadius;
            if (distSq <= collideRadius * collideRadius)
            {
                mPlayer->applyDamage(it->damage);
                it = mSpreadProjectiles.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void Level1::drawSpreadProjectiles() const
{
    if (mSpreadProjectiles.empty())
    {
        return;
    }
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    Rectangle sprite = rm.getSpriteRect(tags::SPREDBALL);
    if (!atlas || sprite.width <= 0.0f || sprite.height <= 0.0f)
    {
        for (const auto &proj : mSpreadProjectiles)
        {
            DrawCircleV(proj.position, proj.radius, Fade(ORANGE, 0.9f));
        }
        return;
    }

    const int frameCount = std::max(level1_consts::SPREAD_BALL_FRAMES, 1);
    const float frameWidth = sprite.width / static_cast<float>(frameCount);

    for (const auto &proj : mSpreadProjectiles)
    {
        const int frameIndex = std::clamp(proj.frame, 0, frameCount - 1);
        Rectangle src = {
            sprite.x + frameWidth * static_cast<float>(frameIndex),
            sprite.y,
            frameWidth,
            sprite.height
        };
        Rectangle dest = {
            proj.position.x,
            proj.position.y,
            frameWidth,
            sprite.height * 2.0f
        };
        Vector2 origin = { dest.width * 0.5f, dest.height * 0.5f };
        DrawTexturePro(*atlas, src, dest, origin, proj.angle * RAD2DEG, WHITE);
    }
}

void Level1::playSwingSFX() const
{
    playRandomSFX(SFX_MELEE_SWING, SFX_MELEE_SWING_COUNT);
}

void Level1::playMeleeHitSFX() const
{
    playRandomSFX(SFX_MELEE_HIT, SFX_MELEE_HIT_COUNT);
}

void Level1::playThrowSFX() const
{
    playRandomSFX(SFX_THROW, SFX_THROW_COUNT);
}

void Level1::playGoldPickupSFX() const
{
    playRandomSFX(SFX_GOLD_PICKUP, SFX_GOLD_PICKUP_COUNT);
}

void Level1::playPotionSFX() const
{
    playRandomSFX(SFX_POTION, SFX_POTION_COUNT);
}

void Level1::playPurchaseSFX(bool success) const
{
    if (!success)
    {
        return;
    }
    playRandomSFX(SFX_PURCHASE_SUCCESS, SFX_PURCHASE_SUCCESS_COUNT);
}

void Level1::playEnemyDeathSFX() const
{
    playRandomSFX(SFX_ENEMY_DEATH, SFX_ENEMY_DEATH_COUNT);
}
