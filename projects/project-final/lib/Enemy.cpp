#include "Enemy.h"

#include <algorithm>
#include <cmath>

Enemy::Enemy(Vector2 position, float moveSpeed, float detectionRadius)
    : Entity(),
      mMoveSpeed(moveSpeed),
      mDetectionRadius(detectionRadius),
      mMaxHealth(EnemyConstants::DEFAULT_HEALTH),
      mHealth(EnemyConstants::DEFAULT_HEALTH)
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *texture = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (texture)
    {
        setTexture(*texture);
        setOwnsTexture(false);
    }

    setPosition(position);
    setIsActive(true);
    setCanCollide(true);
    setScale({EnemyConstants::DEFAULT_HEIGHT, EnemyConstants::DEFAULT_HEIGHT});
    setColliderDimensions({
        EnemyConstants::DEFAULT_HEIGHT * EnemyConstants::COLLIDER_WIDTH_RATIO,
        EnemyConstants::DEFAULT_HEIGHT * EnemyConstants::COLLIDER_HEIGHT_RATIO
    });
}

void Enemy::update(float deltaTime,
                   Entity *player,
                   Map *map,
                   const std::vector<Entity*> &collidableEntities)
{
    if (!getIsActive())
    {
        return;
    }

    updateBehaviour(deltaTime, player);
    Entity::update(deltaTime, player, map, collidableEntities);
}

void Enemy::applySpriteRect(const Rectangle &spriteRect,
                            float desiredPixelHeight,
                            float colliderWidthRatio,
                            float colliderHeightRatio)
{
    if (spriteRect.width <= 0.0f || spriteRect.height <= 0.0f)
    {
        return;
    }

    const float targetHeight = desiredPixelHeight > 0.0f
                               ? desiredPixelHeight
                               : spriteRect.height;
    const float scaleFactor = targetHeight / spriteRect.height;

    Vector2 spriteSize = {
        spriteRect.width * scaleFactor,
        spriteRect.height * scaleFactor
    };

    setScale(spriteSize);
    setCustomSourceRect(spriteRect);

    Vector2 colliderSize = {
        spriteSize.x * colliderWidthRatio,
        spriteSize.y * colliderHeightRatio
    };
    setColliderDimensions(colliderSize);
}

bool Enemy::isPlayerWithinRange(Entity *player) const
{
    if (!player)
    {
        return false;
    }

    Vector2 toPlayer = {
        player->getPosition().x - getPosition().x,
        player->getPosition().y - getPosition().y
    };

    const float distanceSquared = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y;
    const float radiusSquared = mDetectionRadius * mDetectionRadius;
    return distanceSquared <= radiusSquared;
}

void Enemy::setMaxHealth(float health)
{
    mMaxHealth = std::max(health, 1.0f);
    mHealth = std::min(mHealth, mMaxHealth);
}

void Enemy::setHealth(float health)
{
    mHealth = std::clamp(health, 0.0f, mMaxHealth);
    if (mHealth <= 0.0f)
    {
        setIsActive(false);
        setCanCollide(false);
    }
}

bool Enemy::applyDamage(float amount)
{
    if (amount <= 0.0f || !getIsActive())
    {
        return false;
    }

    setHealth(mHealth - amount);
    if (mHealth <= 0.0f)
    {
        LOG_INFO(TextFormat("Enemy[%p] defeated", this));
        return true;
    }
    return false;
}

