#include "goldcoin.h"

#include <cmath>

namespace
{
    constexpr const char *COIN_SPRITE_TAG = "GOLDCOIN";
    constexpr float COIN_TARGET_HEIGHT = 22.0f;
    constexpr float COIN_COLLIDER_RATIO = 0.6f;
    constexpr float COIN_HOVER_AMPLITUDE = 3.5f;
    constexpr float COIN_HOVER_SPEED = 2.6f;
}

GoldCoin::GoldCoin(Vector2 position)
    : Entity(),
      mBasePosition(position)
{
    configureSprite();
    setPosition(position);
    setCanCollide(false);
    setIsActive(true);
}

void GoldCoin::update(float deltaTime,
                      Entity *player,
                      Map *map,
                      const std::vector<Entity*> &collidableEntities)
{
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive())
    {
        return;
    }

    mElapsed += deltaTime;
    const float bob = sinf(mElapsed * COIN_HOVER_SPEED) * COIN_HOVER_AMPLITUDE;
    setPosition({mBasePosition.x, mBasePosition.y + bob});
}

void GoldCoin::configureSprite()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!atlas)
    {
        LOG_WARNING("GoldCoin: missing world atlas texture");
        setScale({COIN_TARGET_HEIGHT, COIN_TARGET_HEIGHT});
        setColliderDimensions({COIN_TARGET_HEIGHT * COIN_COLLIDER_RATIO,
                               COIN_TARGET_HEIGHT * COIN_COLLIDER_RATIO});
        return;
    }

    setTexture(*atlas);
    setOwnsTexture(false);

    Rectangle spriteRect = rm.getSpriteRect(COIN_SPRITE_TAG);
    if (spriteRect.width <= 0.0f || spriteRect.height <= 0.0f)
    {
        LOG_WARNING(TextFormat("GoldCoin: sprite '%s' missing from atlas", COIN_SPRITE_TAG));
        setScale({COIN_TARGET_HEIGHT, COIN_TARGET_HEIGHT});
        setColliderDimensions({COIN_TARGET_HEIGHT * COIN_COLLIDER_RATIO,
                               COIN_TARGET_HEIGHT * COIN_COLLIDER_RATIO});
        return;
    }

    const float scaleFactor = COIN_TARGET_HEIGHT / spriteRect.height;
    Vector2 spriteSize = {
        spriteRect.width * scaleFactor,
        COIN_TARGET_HEIGHT
    };
    setScale(spriteSize);
    setCustomSourceRect(spriteRect);

    Vector2 colliderSize = {
        spriteSize.x * COIN_COLLIDER_RATIO,
        spriteSize.y * COIN_COLLIDER_RATIO
    };
    setColliderDimensions(colliderSize);
}
