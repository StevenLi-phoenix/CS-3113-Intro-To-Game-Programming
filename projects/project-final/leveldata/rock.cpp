#include "rock.h"
#include <algorithm>
#include "../lib/ResourceManager.h"

Rock::Rock(Vector2 position,
           const Rectangle &spriteRect,
           float targetHeight,
           float colliderHeightRatio,
           float colliderWidthRatio)
    : Entity()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *texture = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!texture)
    {
        LOG_WARNING("Rock: missing world atlas texture");
        setIsActive(false);
        setCanCollide(false);
        return;
    }

    Rectangle resolvedRect = spriteRect;
    if (resolvedRect.width <= 0.0f || resolvedRect.height <= 0.0f)
    {
        resolvedRect = {0.0f, 0.0f, 32.0f, 32.0f};
    }

    setTexture(*texture);
    setOwnsTexture(false);
    setCustomSourceRect(resolvedRect);

    const float sourceHeight = std::max(resolvedRect.height, 1.0f);
    const float desiredHeight = std::max(targetHeight, sourceHeight);
    const float scaleFactor = desiredHeight / sourceHeight;
    Vector2 visualSize = {
        resolvedRect.width * scaleFactor,
        desiredHeight
    };
    setScale(visualSize);

    const float clampedHeightRatio = std::clamp(colliderHeightRatio, 0.25f, 0.95f);
    const float clampedWidthRatio = std::clamp(colliderWidthRatio, 0.25f, 1.0f);
    Vector2 colliderSize = {
        visualSize.x * clampedWidthRatio,
        visualSize.y * clampedHeightRatio
    };
    setColliderDimensions(colliderSize);

    const float offsetY = (colliderSize.y - visualSize.y) * 0.5f;
    setTextureOffset({0.0f, offsetY});
    setPosition(position);

    setIsActive(true);
    setCanCollide(true);
}

void Rock::update(float deltaTime,
                  Entity *player,
                  Map *map,
                  const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;
    if (!getIsActive())
    {
        return;
    }
}


