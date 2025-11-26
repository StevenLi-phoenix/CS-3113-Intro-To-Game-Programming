#include "level1.h"
#include "level1_consts.h"
#include "../lib/ResourceManager.h"

using namespace level1_consts;

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

    for (const auto &proj : mSpreadProjectiles)
    {
        Rectangle dest = {
            proj.position.x,
            proj.position.y,
            sprite.width,
            sprite.height * 2.0f
        };
        Vector2 origin = { dest.width * 0.5f, dest.height * 0.5f };
        DrawTexturePro(*atlas, sprite, dest, origin, proj.angle * RAD2DEG, WHITE);
    }
}
