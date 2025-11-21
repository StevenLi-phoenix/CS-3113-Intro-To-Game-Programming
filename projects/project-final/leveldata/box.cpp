#include "box.h"

namespace
{
    constexpr const char *BOX_SPRITE_TAG = "SMALLBOX";
    constexpr float BOX_TARGET_HEIGHT = 40.0f;
    constexpr float BOX_COLLIDER_RATIO = 0.8f;
}

Box::Box(Vector2 position)
    : Entity()
{
    configureSprite();
    setPosition(position);
    setCanCollide(false);
    setIsActive(true);
}

void Box::markCollected()
{
    if (mCollected)
    {
        return;
    }
    mCollected = true;
    setIsActive(false);
    setCanCollide(false);
}

void Box::update(float deltaTime,
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
    // Boxes are static; nothing to update per frame yet.
}

void Box::configureSprite()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!atlas)
    {
        LOG_WARNING("Box: missing world atlas texture");
        return;
    }

    setTexture(*atlas);
    setOwnsTexture(false);

    Rectangle spriteRect = rm.getSpriteRect(BOX_SPRITE_TAG);
    if (spriteRect.width <= 0.0f || spriteRect.height <= 0.0f)
    {
        LOG_WARNING(TextFormat("Box: sprite '%s' missing from atlas", BOX_SPRITE_TAG));
        Vector2 fallbackSize = {32.0f, 32.0f};
        setScale(fallbackSize);
        setColliderDimensions({fallbackSize.x * BOX_COLLIDER_RATIO,
                               fallbackSize.y * BOX_COLLIDER_RATIO});
        return;
    }

    const float scaleFactor = BOX_TARGET_HEIGHT / spriteRect.height;
    Vector2 spriteSize = {
        spriteRect.width * scaleFactor,
        BOX_TARGET_HEIGHT
    };
    setScale(spriteSize);
    setCustomSourceRect(spriteRect);

    Vector2 colliderSize = {
        spriteSize.x * BOX_COLLIDER_RATIO,
        spriteSize.y * BOX_COLLIDER_RATIO
    };
    setColliderDimensions(colliderSize);
}

