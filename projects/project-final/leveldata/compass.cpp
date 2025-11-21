#include "compass.h"

#include "../lib/ResourceManager.h"

namespace
{
    constexpr const char *COMPASS_TAG = "COMPASS";
}

Compass::Compass()
    : UIBase()
{
    setIsActive(true);
    setCanCollide(false);
    loadIcon();
}

void Compass::loadIcon()
{
    ResourceManager &rm = ResourceManager::instance();
    mIcon = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    mIconSource = rm.getSpriteRect(COMPASS_TAG);
    if (!mIcon || mIconSource.width <= 0.0f || mIconSource.height <= 0.0f)
    {
        mIcon = nullptr;
        mIconSource = {0.0f, 0.0f, 0.0f, 0.0f};
    }
}

void Compass::update(float deltaTime,
                     Entity *player,
                     Map *map,
                     const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)map;
    (void)collidableEntities;
    if (player)
    {
        mPlayer = player;
    }
}

void Compass::render()
{
    if (!getIsActive() || !mTarget || !mPlayer || !mCamera)
    {
        return;
    }

    const Vector2 playerPos = mPlayer->getPosition();
    const Vector2 targetPos = mTarget->getPosition();
    Vector2 direction = { targetPos.x - playerPos.x, targetPos.y - playerPos.y };
    const float distSq = direction.x * direction.x + direction.y * direction.y;
    if (distSq < 1.0f)
    {
        return;
    }

    direction = Vector2Normalize(direction);

    Vector2 playerScreen = GetWorldToScreen2D(playerPos, *mCamera);
    const Vector2 arrowTip = {
        playerScreen.x + direction.x * mArrowRadius,
        playerScreen.y + direction.y * mArrowRadius
    };
    const Vector2 arrowBase = {
        playerScreen.x + direction.x * (mArrowRadius - 24.0f),
        playerScreen.y + direction.y * (mArrowRadius - 24.0f)
    };
    const Vector2 perpendicular = { -direction.y, direction.x };
    const Vector2 leftWing = {
        arrowBase.x + perpendicular.x * 8.0f,
        arrowBase.y + perpendicular.y * 8.0f
    };
    const Vector2 rightWing = {
        arrowBase.x - perpendicular.x * 8.0f,
        arrowBase.y - perpendicular.y * 8.0f
    };

    DrawLineEx(playerScreen, arrowTip, mArrowThickness, mArrowColor);
    DrawTriangle(arrowTip, leftWing, rightWing, Fade(mArrowColor, 0.9f));

    if (mIcon && mIconSource.width > 0.0f && mIconSource.height > 0.0f)
    {
        Rectangle iconRect = {
            playerScreen.x - mArrowRadius - 40.0f,
            playerScreen.y - mArrowRadius - 40.0f,
            32.0f,
            32.0f
        };
        DrawRectangleRounded(iconRect, 0.3f, 6, mBackground);
        Rectangle dest = {
            iconRect.x + 4.0f,
            iconRect.y + 4.0f,
            iconRect.width - 8.0f,
            iconRect.height - 8.0f
        };
        DrawTexturePro(*mIcon, mIconSource, dest, {0.0f, 0.0f}, 0.0f, WHITE);
    }
}
