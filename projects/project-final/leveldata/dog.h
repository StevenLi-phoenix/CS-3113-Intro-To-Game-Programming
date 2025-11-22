#ifndef DOG_H
#define DOG_H

#include "../lib/Enemy.h"

namespace DogConstants
{
    constexpr int VARIANT_COUNT = 4;
    constexpr float DEFAULT_HEIGHT = 28.0f;
    constexpr float MIN_HEIGHT = 24.0f;
    constexpr float MAX_HEIGHT = 36.0f;
    constexpr float CHASE_SPEED = 45.0f;
    constexpr float PATROL_SPEED = 10.0f;
    constexpr float DETECTION_RADIUS = 400.0f;
    constexpr float COLLIDER_WIDTH_RATIO = 0.8f;
    constexpr float COLLIDER_HEIGHT_RATIO = 0.5f;
    constexpr float PATROL_RADIUS = 80.0f;
    constexpr float PATROL_RETARGET_TIME = 2.5f;
    constexpr float CHASE_EXIT_GRACE = 1.4f;
    constexpr float ATTACK_RANGE = 1.0f;
    constexpr float ATTACK_DAMAGE = 1.0f;
    constexpr float ATTACK_COOLDOWN = 1.0f;
    constexpr float VARIANT_CHASE_SPEEDS[VARIANT_COUNT] = {45.0f, 52.0f, 40.0f, 48.0f};
    constexpr float VARIANT_PATROL_SPEEDS[VARIANT_COUNT] = {10.0f, 12.0f, 8.5f, 11.0f};
    constexpr float BIAS_MAIN_AXIS_WEIGHT = 1.35f;
    constexpr float BIAS_OFF_AXIS_WEIGHT = 0.75f;
    constexpr const char *SPRITE_TAGS[VARIANT_COUNT] = {"DOG1", "DOG2", "DOG3", "DOG4"};
}

class Dog : public Enemy
{
public:
    Dog(Vector2 position,
        int variant = 0,
        float desiredHeightPixels = DogConstants::DEFAULT_HEIGHT);
protected:
    void updateBehaviour(float deltaTime, Entity *player) override;

private:
    enum class MovementBias
    {
        Horizontal,
        Vertical
    };

    Rectangle resolveSpriteRect(int variant) const;
    void updatePatrol(float deltaTime);
    void resetChaseState();
    Vector2 randomPatrolTarget() const;
    void attemptAttack(Entity *player, float distanceToPlayer);
    Vector2 applyMovementBias(const Vector2 &direction) const;

    int mVariant;
    float mDesiredHeight;
    MovementBias mMovementBias = MovementBias::Horizontal;
    float mChaseSpeed = DogConstants::CHASE_SPEED;
    float mPatrolSpeed = DogConstants::PATROL_SPEED;
    bool mIsChasing = false;
    Vector2 mPatrolHome{};
    Vector2 mPatrolTarget{};
    float mPatrolTimer = 0.0f;
    bool mHasPatrolTarget = false;
    float mChaseLoseTimer = 0.0f;
    float mAttackCooldownTimer = 0.0f;
};

#endif // DOG_H
