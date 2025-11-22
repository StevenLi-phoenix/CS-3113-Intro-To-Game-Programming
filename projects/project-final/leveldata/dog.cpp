#include "dog.h"
#include "player.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
int normalizeVariantIndex(int variant)
{
    if (DogConstants::VARIANT_COUNT <= 0)
    {
        return 0;
    }
    const int mod = variant % DogConstants::VARIANT_COUNT;
    return (mod + DogConstants::VARIANT_COUNT) % DogConstants::VARIANT_COUNT;
}

float resolveVariantScalar(const float values[], int count, int variant, float fallback)
{
    if (!values || count <= 0)
    {
        return fallback;
    }
    if (variant < 0 || variant >= count)
    {
        return fallback;
    }
    return values[variant];
}
} // namespace

Dog::Dog(Vector2 position, int variant, float desiredHeightPixels)
    : Enemy(position,
            resolveVariantScalar(DogConstants::VARIANT_PATROL_SPEEDS,
                                 DogConstants::VARIANT_COUNT,
                                 normalizeVariantIndex(variant),
                                 DogConstants::PATROL_SPEED),
            DogConstants::DETECTION_RADIUS),
      mVariant(normalizeVariantIndex(variant)),
      mDesiredHeight(std::clamp(desiredHeightPixels, DogConstants::MIN_HEIGHT, DogConstants::MAX_HEIGHT)),
      mMovementBias((mVariant % 2 == 0) ? MovementBias::Horizontal : MovementBias::Vertical),
      mChaseSpeed(resolveVariantScalar(DogConstants::VARIANT_CHASE_SPEEDS,
                                       DogConstants::VARIANT_COUNT,
                                       mVariant,
                                       DogConstants::CHASE_SPEED)),
      mPatrolSpeed(resolveVariantScalar(DogConstants::VARIANT_PATROL_SPEEDS,
                                        DogConstants::VARIANT_COUNT,
                                        mVariant,
                                        DogConstants::PATROL_SPEED))
{
    Rectangle spriteRect = resolveSpriteRect(mVariant);
    applySpriteRect(spriteRect,
                    mDesiredHeight,
                    DogConstants::COLLIDER_WIDTH_RATIO,
                    DogConstants::COLLIDER_HEIGHT_RATIO);
    setTextureFacesLeft(true);

    setVelocity({0.0f, 0.0f});
    mPatrolHome = position;
    mPatrolTarget = position;
    mHasPatrolTarget = false;
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
    if (mAttackCooldownTimer > 0.0f)
    {
        mAttackCooldownTimer = std::max(0.0f, mAttackCooldownTimer - deltaTime);
    }

    bool playerDetected = playerValid && isPlayerWithinRange(player);
    if (playerDetected)
    {
        mChaseLoseTimer = DogConstants::CHASE_EXIT_GRACE;
    }
    else if (mChaseLoseTimer > 0.0f)
    {
        mChaseLoseTimer = std::max(0.0f, mChaseLoseTimer - deltaTime);
    }

    const bool shouldChase = playerValid && (playerDetected || mChaseLoseTimer > 0.0f);

    if (shouldChase)
    {
        tickPathCooldown(deltaTime);
        refreshPathTo(player->getPosition(), false);

        Vector2 targetPosition = resolvePathTarget(player->getPosition());

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
            Vector2 biasedDirection = applyMovementBias(direction);
            Vector2 velocity = {
                biasedDirection.x * mChaseSpeed,
                biasedDirection.y * mChaseSpeed
            };
            setVelocity(velocity);

            if (biasedDirection.x > 0.1f)
            {
                setIsHorizontalFlipped(false);
            }
            else if (biasedDirection.x < -0.1f)
            {
                setIsHorizontalFlipped(true);
            }
        }
        const float playerDistance = Vector2Distance(player->getPosition(), getPosition());
        attemptAttack(player, playerDistance);
        if (detectPathStall(deltaTime, distance))
        {
            refreshPathTo(player->getPosition(), true);
        }
    }
    else
    {
        if (mIsChasing)
        {
            LOG_INFO(TextFormat("Dog[%p] lost target at pos=(%.1f,%.1f)", this, getPosition().x, getPosition().y));
        }
        resetChaseState();
        updatePatrol(deltaTime);
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


void Dog::updatePatrol(float deltaTime)
{
    mPatrolTimer -= deltaTime;
    if (!mHasPatrolTarget || mPatrolTimer <= 0.0f || hasReachedTarget(mPatrolTarget, getPathNodeReachedRadius()))
    {
        mPatrolTarget = randomPatrolTarget();
        mHasPatrolTarget = true;
        mPatrolTimer = DogConstants::PATROL_RETARGET_TIME;
    }

    Vector2 toTarget = {
        mPatrolTarget.x - getPosition().x,
        mPatrolTarget.y - getPosition().y
    };
    const float distance = Vector2Length(toTarget);
    if (distance > 1.0f)
    {
        Vector2 direction = {
            toTarget.x / distance,
            toTarget.y / distance
        };
        direction = applyMovementBias(direction);
        Vector2 velocity = {
            direction.x * mPatrolSpeed,
            direction.y * mPatrolSpeed
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
    else
    {
        Vector2 velocity = getVelocity();
        velocity.x *= 0.85f;
        velocity.y *= 0.85f;
        if (std::fabs(velocity.x) < 0.05f) velocity.x = 0.0f;
        if (std::fabs(velocity.y) < 0.05f) velocity.y = 0.0f;
        setVelocity(velocity);
    }
}

void Dog::resetChaseState()
{
    if (hasActivePath() && isDebugMode())
    {
        LOG_DEBUG(TextFormat("Dog[%p] cleared path after losing target", this));
    }
    mIsChasing = false;
    resetPathState();
    setPathCooldown(0.0f);
    mChaseLoseTimer = 0.0f;
}

Vector2 Dog::randomPatrolTarget() const
{
    const float angle = (static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f) * 2.0f * PI;
    const float radius = (static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f) * DogConstants::PATROL_RADIUS;
    return {
        mPatrolHome.x + cosf(angle) * radius,
        mPatrolHome.y + sinf(angle) * radius
    };
}

void Dog::attemptAttack(Entity *player, float distanceToPlayer)
{
    if (!player || mAttackCooldownTimer > 0.0f)
    {
        return;
    }

    Player *playerEntity = dynamic_cast<Player*>(player);
    if (!playerEntity || playerEntity->isDead())
    {
        return;
    }

    const float dogRadius = std::max(getColliderDimensions().x, getColliderDimensions().y) * 0.5f;
    const float playerRadius = std::max(playerEntity->getColliderDimensions().x,
                                        playerEntity->getColliderDimensions().y) * 0.5f;
    const float effectiveRange = DogConstants::ATTACK_RANGE + dogRadius + playerRadius;

    if (distanceToPlayer > effectiveRange)
    {
        return;
    }

    const bool applied = playerEntity->applyDamage(DogConstants::ATTACK_DAMAGE);
    if (applied && isDebugMode())
    {
        LOG_INFO(TextFormat("Dog[%p] hit player damage=%.1f", this, DogConstants::ATTACK_DAMAGE));
    }

    mAttackCooldownTimer = DogConstants::ATTACK_COOLDOWN;
}

Vector2 Dog::applyMovementBias(const Vector2 &direction) const
{
    Vector2 biased = direction;
    if (mMovementBias == MovementBias::Horizontal)
    {
        biased.x *= DogConstants::BIAS_MAIN_AXIS_WEIGHT;
        biased.y *= DogConstants::BIAS_OFF_AXIS_WEIGHT;
    }
    else
    {
        biased.y *= DogConstants::BIAS_MAIN_AXIS_WEIGHT;
        biased.x *= DogConstants::BIAS_OFF_AXIS_WEIGHT;
    }

    const float magnitude = Vector2Length(biased);
    if (magnitude <= std::numeric_limits<float>::epsilon())
    {
        return direction;
    }

    biased.x /= magnitude;
    biased.y /= magnitude;
    return biased;
}
