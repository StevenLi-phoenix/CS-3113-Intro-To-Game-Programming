#ifndef ENEMY_H
#define ENEMY_H

#include <algorithm>
#include <vector>
#include "Entity.h"
#include "ResourceManager.h"
#include "NavMap.h"

namespace EnemyConstants
{
    constexpr float DEFAULT_HEIGHT = 32.0f;
    constexpr float DEFAULT_SPEED = 55.0f;
    constexpr float DEFAULT_DETECTION_RADIUS = 460.0f;
    constexpr float DEFAULT_HEALTH = 4.0f;
    constexpr float COLLIDER_WIDTH_RATIO = 0.7f;
    constexpr float COLLIDER_HEIGHT_RATIO = 0.45f;
    constexpr float PATH_REFRESH_INTERVAL = 0.35f;
    constexpr float PATH_NODE_REACHED_RADIUS = 12.0f;
    constexpr float PATH_STUCK_MOVE_EPS = 2.0f;
    constexpr float PATH_STUCK_REPATH_TIME = 0.6f;
    constexpr float PATH_PROGRESS_EPS = 4.0f;
    constexpr float PATH_PROGRESS_TIMEOUT = 0.8f;
    constexpr float PATH_FAILURE_COOLDOWN_SCALE = 0.5f;

    struct PathSettings
    {
        float refreshInterval = PATH_REFRESH_INTERVAL;
        float nodeReachedRadius = PATH_NODE_REACHED_RADIUS;
        float stuckMoveEps = PATH_STUCK_MOVE_EPS;
        float stuckRepathTime = PATH_STUCK_REPATH_TIME;
        float progressEps = PATH_PROGRESS_EPS;
        float progressTimeout = PATH_PROGRESS_TIMEOUT;
        float failureCooldownScale = PATH_FAILURE_COOLDOWN_SCALE;
    };

    constexpr PathSettings DEFAULT_PATH_SETTINGS = {
        PATH_REFRESH_INTERVAL,
        PATH_NODE_REACHED_RADIUS,
        PATH_STUCK_MOVE_EPS,
        PATH_STUCK_REPATH_TIME,
        PATH_PROGRESS_EPS,
        PATH_PROGRESS_TIMEOUT,
        PATH_FAILURE_COOLDOWN_SCALE
    };
}

class Enemy : public Entity
{
public:
    Enemy(Vector2 position,
          float moveSpeed = EnemyConstants::DEFAULT_SPEED,
          float detectionRadius = EnemyConstants::DEFAULT_DETECTION_RADIUS);

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;

    void setNavMap(const NavMap *navMap) { mNavMap = navMap; }
    const NavMap* getNavMap() const { return mNavMap; }
    bool hasActivePath() const;
    std::vector<Vector2> activePathPoints() const;

    void setMaxHealth(float health);
    void setHealth(float health);
    float getHealth() const { return mHealth; }
    float getMaxHealth() const { return mMaxHealth; }
    bool isDead() const { return mHealth <= 0.0f; }
    bool applyDamage(float amount);

protected:
    void applySpriteRect(const Rectangle &spriteRect,
                         float desiredPixelHeight = EnemyConstants::DEFAULT_HEIGHT,
                         float colliderWidthRatio = EnemyConstants::COLLIDER_WIDTH_RATIO,
                         float colliderHeightRatio = EnemyConstants::COLLIDER_HEIGHT_RATIO);
    bool isPlayerWithinRange(Entity *player) const;
    void setPathSettings(const EnemyConstants::PathSettings &settings);
    void tickPathCooldown(float deltaTime);
    bool refreshPathTo(const Vector2 &targetPosition,
                       const std::vector<Entity*> &neighbours,
                       bool forceRebuild);
    Vector2 resolvePathTarget(const Vector2 &fallbackTarget);
    bool detectPathStall(float deltaTime, float distanceToTarget);
    bool hasReachedTarget(const Vector2 &target, float radius) const;
    void resetPathState();
    float getPathNodeReachedRadius() const { return mPathSettings.nodeReachedRadius; }
    void setPathCooldown(float cooldown) { mPathCooldown = std::max(0.0f, cooldown); }
    float getColliderClearanceRadius() const;

    float getMoveSpeed() const { return mMoveSpeed; }
    void setMoveSpeed(float speed) { mMoveSpeed = speed; }
    float getDetectionRadius() const { return mDetectionRadius; }
    void setDetectionRadius(float radius) { mDetectionRadius = radius; }

    virtual void updateBehaviour(float deltaTime,
                                 Entity *player,
                                 const std::vector<Entity*> &collidableEntities) {}

private:
    float mMoveSpeed;
    float mDetectionRadius;
    const NavMap *mNavMap = nullptr;
    float mMaxHealth;
    float mHealth;
    EnemyConstants::PathSettings mPathSettings = EnemyConstants::DEFAULT_PATH_SETTINGS;
    bool mHasPath = false;
    std::vector<Vector2> mCurrentPath;
    size_t mCurrentPathIndex = 1;
    float mPathCooldown = 0.0f;
    Vector2 mLastPathPosition{};
    float mStuckTimer = 0.0f;
    float mProgressTimer = 0.0f;
    float mLastDistanceToTarget = 0.0f;
    bool mHasLastDistance = false;
};

#endif // ENEMY_H
