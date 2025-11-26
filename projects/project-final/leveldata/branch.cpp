#include "branch.h"

namespace
{
    constexpr const char *BRANCH_SPRITE_TAG = "BRANCH";
    constexpr float BRANCH_TARGET_HEIGHT = 18.0f;
    constexpr float COLLIDER_HEIGHT_RATIO = 0.6f;
}

Branch::Branch(const Vector2 &start,
               const Vector2 &direction,
               float travelDistance,
               float speed,
               float damage,
               bool useShuriken)
    : Entity(),
    mDirection(direction),
    mTravelDistance(std::max(travelDistance, branch::MIN_THROW_DISTANCE)),
    mSpeed(std::max(speed, 1.0f)),
    mTraveled(0.0f),
    mDamage(std::max(damage, 0.0f)),
    mSpent(false),
    mRecoverable(false),
    mCollected(false),
    mUseShuriken(useShuriken)
{
    const float length = Vector2Length(mDirection);
    if (length <= 0.0001f)
    {
        mDirection = {1.0f, 0.0f};
    }
    else
    {
        mDirection = Vector2Normalize(mDirection);
    }

    setPosition(start);
    setCanCollide(false);
    setIsActive(true);
    configureSprite();
    setVelocity({mDirection.x * mSpeed, mDirection.y * mSpeed});
}

void Branch::update(float deltaTime,
                    Entity *player,
                    Map *map,
                    const std::vector<Entity*> &collidableEntities)
{
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (mSpent || !getIsActive())
    {
        return;
    }

    Entity::update(deltaTime, player, map, collidableEntities);

    mTraveled += mSpeed * deltaTime;
    if (mTraveled >= mTravelDistance)
    {
        markSpent();
    }
}

void Branch::markSpent()
{
    if (mSpent)
    {
        return;
    }
    mSpent = true;
    setVelocity({0.0f, 0.0f});
    if (mRecoverable)
    {
        setIsActive(true);
        setCanCollide(false);
    }
    else
    {
        setIsActive(false);
    }
}

void Branch::markCollected()
{
    if (mCollected)
    {
        return;
    }
    mCollected = true;
    mSpent = true;
    setIsActive(false);
    setCanCollide(false);
}

void Branch::configureSprite()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!atlas)
    {
        LOG_WARNING("Branch: missing world atlas texture");
        return;
    }

    setTexture(*atlas);
    setOwnsTexture(false);

    const char *tag = mUseShuriken ? "SMALLSHURIKEN" : BRANCH_SPRITE_TAG;
    Rectangle spriteRect = rm.getSpriteRect(tag);
    if (spriteRect.width <= 0.0f || spriteRect.height <= 0.0f)
    {
        LOG_WARNING(TextFormat("Branch: missing sprite rect for tag '%s'", tag));
        Vector2 fallbackSize = {32.0f, 10.0f};
        setScale(fallbackSize);
        setColliderDimensions({fallbackSize.x * 0.7f, fallbackSize.y * COLLIDER_HEIGHT_RATIO});
        return;
    }

    const float scaleFactor = BRANCH_TARGET_HEIGHT / spriteRect.height;
    Vector2 spriteSize = {
        spriteRect.width * scaleFactor,
        BRANCH_TARGET_HEIGHT
    };
    setScale(spriteSize);
    setCustomSourceRect(spriteRect);
    setColliderDimensions({spriteSize.x * 0.7f, spriteSize.y * COLLIDER_HEIGHT_RATIO});
}
