#include "Enemy.h"

#include <algorithm>
#include <cmath>

Enemy::Enemy(Vector2 position, float moveSpeed, float detectionRadius)
    : Entity(),
      mMoveSpeed(moveSpeed),
      mDetectionRadius(detectionRadius),
      mMaxHealth(EnemyConstants::DEFAULT_HEALTH),
      mHealth(EnemyConstants::DEFAULT_HEALTH),
      mPathSettings(EnemyConstants::DEFAULT_PATH_SETTINGS)
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

    mLastPathPosition = position;
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

void Enemy::render()
{
    if (!getIsActive())
    {
        return;
    }

    Entity::render();

    const float maxHealth = std::max(mMaxHealth, 0.001f);
    const float healthRatio = std::clamp(mHealth / maxHealth, 0.0f, 1.0f);
    const Vector2 scale = getScale();
    const float barWidth = std::max(scale.x * 0.6f, 24.0f);
    const float barHeight = 4.0f;
    const float verticalOffset = -scale.y * 0.55f;
    const Vector2 position = getPosition();

    Rectangle barBackground = {
        position.x - barWidth * 0.5f,
        position.y + verticalOffset - barHeight,
        barWidth,
        barHeight
    };

    Rectangle barFill = barBackground;
    barFill.width = barBackground.width * healthRatio;

    DrawRectangleRounded(barBackground, 0.6f, 4, Fade(BLACK, 0.7f));
    DrawRectangleRec(barFill, DARKGREEN);
    DrawRectangleLinesEx(barBackground, 0.6f, Fade(RAYWHITE, 0.8f));
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

void Enemy::setPathSettings(const EnemyConstants::PathSettings &settings)
{
    mPathSettings.refreshInterval = std::max(0.0f, settings.refreshInterval);
    mPathSettings.nodeReachedRadius = std::max(0.0f, settings.nodeReachedRadius);
    mPathSettings.stuckMoveEps = std::max(0.0f, settings.stuckMoveEps);
    mPathSettings.stuckRepathTime = std::max(0.0f, settings.stuckRepathTime);
    mPathSettings.progressEps = std::max(0.0f, settings.progressEps);
    mPathSettings.progressTimeout = std::max(0.0f, settings.progressTimeout);
    mPathSettings.failureCooldownScale = std::max(0.0f, settings.failureCooldownScale);
}

void Enemy::tickPathCooldown(float deltaTime)
{
    if (mPathCooldown > 0.0f)
    {
        mPathCooldown = std::max(0.0f, mPathCooldown - deltaTime);
    }
}

bool Enemy::refreshPathTo(const Vector2 &targetPosition, bool forceRebuild)
{
    const NavMap *navMap = getNavMap();
    if (!navMap)
    {
        resetPathState();
        return false;
    }

    if (!forceRebuild)
    {
        if (hasActivePath() && mPathCooldown > 0.0f)
        {
            return false;
        }
    }

    const double tRequestStart = GetTime();
    const std::vector<Vector2> newPath = navMap->findPath(getPosition(), targetPosition);
    const double elapsedMs = (GetTime() - tRequestStart) * 1000.0;
    if (newPath.size() >= 2)
    {
        mCurrentPath = newPath;
        mCurrentPathIndex = 1;
        mHasPath = true;
        mPathCooldown = mPathSettings.refreshInterval;
        if (isDebugMode())
        {
            LOG_INFO(TextFormat("Enemy[%p] path success nodes=%zu time=%.2fms force=%s from=(%.1f,%.1f) to=(%.1f,%.1f)",
                                this,
                                mCurrentPath.size(),
                                elapsedMs,
                                forceRebuild ? "true" : "false",
                                getPosition().x,
                                getPosition().y,
                                targetPosition.x,
                                targetPosition.y));
        }
        return true;
    }

    if (isDebugMode())
    {
        LOG_WARNING(TextFormat("Enemy[%p] path failed nodes=%zu time=%.2fms force=%s from=(%.1f,%.1f) to=(%.1f,%.1f)",
                               this,
                               newPath.size(),
                               elapsedMs,
                               forceRebuild ? "true" : "false",
                               getPosition().x,
                               getPosition().y,
                               targetPosition.x,
                               targetPosition.y));
    }
    resetPathState();
    mPathCooldown = mPathSettings.refreshInterval * mPathSettings.failureCooldownScale;
    return false;
}

Vector2 Enemy::resolvePathTarget(const Vector2 &fallbackTarget)
{
    if (mHasPath && mCurrentPathIndex < mCurrentPath.size())
    {
        Vector2 currentTarget = mCurrentPath[mCurrentPathIndex];
        if (hasReachedTarget(currentTarget, mPathSettings.nodeReachedRadius))
        {
            ++mCurrentPathIndex;
            if (mCurrentPathIndex < mCurrentPath.size())
            {
                currentTarget = mCurrentPath[mCurrentPathIndex];
            }
            else
            {
                resetPathState();
                return fallbackTarget;
            }
        }
        return currentTarget;
    }
    return fallbackTarget;
}

bool Enemy::detectPathStall(float deltaTime, float distanceToTarget)
{
    bool requestRepath = false;
    const float movedDistance = Vector2Distance(mLastPathPosition, getPosition());
    if (movedDistance < mPathSettings.stuckMoveEps)
    {
        mStuckTimer += deltaTime;
    }
    else
    {
        mStuckTimer = 0.0f;
    }

    if (!mHasLastDistance)
    {
        mLastDistanceToTarget = distanceToTarget;
        mHasLastDistance = true;
        mProgressTimer = 0.0f;
    }
    else
    {
        if (distanceToTarget > mLastDistanceToTarget - mPathSettings.progressEps)
        {
            mProgressTimer += deltaTime;
        }
        else
        {
            mProgressTimer = 0.0f;
        }
        mLastDistanceToTarget = distanceToTarget;
    }

    if (mStuckTimer >= mPathSettings.stuckRepathTime ||
        mProgressTimer >= mPathSettings.progressTimeout)
    {
        requestRepath = true;
        if (isDebugMode())
        {
            LOG_DEBUG(TextFormat("Enemy[%p] repath due to %s (stuck=%.2f progress=%.2f)",
                                 this,
                                 mStuckTimer >= mPathSettings.stuckRepathTime ? "movement stall" : "no progress",
                                 mStuckTimer,
                                 mProgressTimer));
        }
        mStuckTimer = 0.0f;
        mProgressTimer = 0.0f;
    }

    mLastPathPosition = getPosition();
    return requestRepath;
}

bool Enemy::hasReachedTarget(const Vector2 &target, float radius) const
{
    const Vector2 diff = {
        target.x - getPosition().x,
        target.y - getPosition().y
    };
    const float distanceSq = diff.x * diff.x + diff.y * diff.y;
    return distanceSq <= radius * radius;
}

void Enemy::resetPathState()
{
    mHasPath = false;
    mCurrentPath.clear();
    mCurrentPathIndex = 1;
    mStuckTimer = 0.0f;
    mProgressTimer = 0.0f;
    mHasLastDistance = false;
    mLastPathPosition = getPosition();
}

bool Enemy::hasActivePath() const
{
    return mHasPath && mCurrentPathIndex < mCurrentPath.size();
}

std::vector<Vector2> Enemy::activePathPoints() const
{
    std::vector<Vector2> points;
    if (!hasActivePath())
    {
        return points;
    }

    points.reserve(mCurrentPath.size() - mCurrentPathIndex + 1);
    points.push_back(getPosition());
    for (size_t i = mCurrentPathIndex; i < mCurrentPath.size(); ++i)
    {
        points.push_back(mCurrentPath[i]);
    }
    return points;
}
