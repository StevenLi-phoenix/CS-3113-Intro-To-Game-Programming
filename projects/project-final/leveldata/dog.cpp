#include "dog.h"

#include <algorithm>
#include <cmath>
#include <vector>

Dog::Dog(Vector2 position, int variant, float desiredHeightPixels)
    : Enemy(position, DogConstants::PATROL_SPEED, DogConstants::DETECTION_RADIUS),
      mVariant((variant % DogConstants::VARIANT_COUNT + DogConstants::VARIANT_COUNT) % DogConstants::VARIANT_COUNT),
      mDesiredHeight(std::clamp(desiredHeightPixels, DogConstants::MIN_HEIGHT, DogConstants::MAX_HEIGHT))
{
    Rectangle spriteRect = resolveSpriteRect(mVariant);
    applySpriteRect(spriteRect,
                    mDesiredHeight,
                    DogConstants::COLLIDER_WIDTH_RATIO,
                    DogConstants::COLLIDER_HEIGHT_RATIO);

    setVelocity({0.0f, 0.0f});
    if (isDebugMode())
    {
        LOG_DEBUG(TextFormat("Dog[%p] spawned variant=%d height=%.1f pos=(%.1f,%.1f)",
                             this,
                             mVariant,
                             mDesiredHeight,
                             position.x,
                             position.y));
    }
}

void Dog::updateBehaviour(float deltaTime, Entity *player)
{
    const bool playerValid = player && player->getIsActive();
    const bool playerInRange = playerValid && isPlayerWithinRange(player);

    if (!playerInRange)
    {
        if (mIsChasing)
        {
            LOG_INFO(TextFormat("Dog[%p] lost target at pos=(%.1f,%.1f)", this, getPosition().x, getPosition().y));
        }
        if (mHasPath)
        {
            LOG_DEBUG(TextFormat("Dog[%p] cleared path after losing target", this));
        }
        mIsChasing = false;
        mHasPath = false;
        mCurrentPath.clear();
        mCurrentPathIndex = 1;
        mPathCooldown = 0.0f;

        Vector2 velocity = getVelocity();
        velocity.x *= 0.92f;
        velocity.y *= 0.92f;

        if (std::fabs(velocity.x) < 0.1f) velocity.x = 0.0f;
        if (std::fabs(velocity.y) < 0.1f) velocity.y = 0.0f;
        setVelocity(velocity);
        return;
    }

    if (mPathCooldown > 0.0f)
    {
        mPathCooldown = std::max(0.0f, mPathCooldown - deltaTime);
    }

    const NavMap *navMap = getNavMap();
    if (navMap && (!mHasPath || mPathCooldown <= 0.0f))
    {
        const std::vector<Vector2> newPath = navMap->findPath(getPosition(), player->getPosition());
        if (newPath.size() >= 2)
        {
            mCurrentPath = newPath;
            mCurrentPathIndex = 1;
            if (!mHasPath)
            {
                LOG_DEBUG(TextFormat("Dog[%p] acquired path nodes=%zu", this, mCurrentPath.size()));
            }
            mHasPath = true;
            mPathCooldown = DogConstants::PATH_REFRESH_INTERVAL;
        }
        else if (mHasPath)
        {
            LOG_DEBUG(TextFormat("Dog[%p] lost path (path too short)", this));
            mCurrentPath.clear();
            mCurrentPathIndex = 1;
            mHasPath = false;
            mPathCooldown = DogConstants::PATH_REFRESH_INTERVAL * 0.5f;
        }
    }

    Vector2 targetPosition = player->getPosition();
    if (mHasPath && mCurrentPathIndex < mCurrentPath.size())
    {
        targetPosition = mCurrentPath[mCurrentPathIndex];
        Vector2 toNode = {
            targetPosition.x - getPosition().x,
            targetPosition.y - getPosition().y
        };
        const float nodeRadius = DogConstants::PATH_NODE_REACHED_RADIUS;
        const float nodeRadiusSq = nodeRadius * nodeRadius;
        const float distSq = toNode.x * toNode.x + toNode.y * toNode.y;
        if (distSq <= nodeRadiusSq)
        {
            ++mCurrentPathIndex;
            if (mCurrentPathIndex < mCurrentPath.size())
            {
                targetPosition = mCurrentPath[mCurrentPathIndex];
            }
            else
            {
                mCurrentPath.clear();
                mCurrentPathIndex = 1;
                mHasPath = false;
            }
        }
    }

    if (!mIsChasing)
    {
        LOG_INFO(TextFormat("Dog[%p] started chase target=(%.1f,%.1f)", this, targetPosition.x, targetPosition.y));
    }
    mIsChasing = true;

    Vector2 toTarget = {
        targetPosition.x - getPosition().x,
        targetPosition.y - getPosition().y
    };

    const float distance = Vector2Length(toTarget);
    if (distance > 0.01f)
    {
        Vector2 direction = {
            toTarget.x / distance,
            toTarget.y / distance
        };
        Vector2 velocity = {
            direction.x * DogConstants::CHASE_SPEED,
            direction.y * DogConstants::CHASE_SPEED
        };
        setVelocity(velocity);

        if (direction.x > 0.1f)
        {
            setIsHorizontalFlipped(false);
        }
        else if (direction.x < -0.1f)
        {
            setIsHorizontalFlipped(true);
        }
    }
}

Rectangle Dog::resolveSpriteRect(int variant) const
{
    variant = std::clamp(variant, 0, DogConstants::VARIANT_COUNT - 1);

    ResourceManager &rm = ResourceManager::instance();
    const char *tag = DogConstants::SPRITE_TAGS[variant];
    Rectangle rect = rm.getSpriteRect(tag);
    if (rect.width <= 0.0f || rect.height <= 0.0f)
    {
        return {0.0f, 0.0f, 24.0f, 13.0f};
    }
    return rect;
}


