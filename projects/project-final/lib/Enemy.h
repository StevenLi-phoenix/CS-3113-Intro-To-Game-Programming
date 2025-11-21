#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include "ResourceManager.h"
#include "NavMap.h"

namespace EnemyConstants
{
    constexpr float DEFAULT_HEIGHT = 32.0f;
    constexpr float DEFAULT_SPEED = 35.0f;
    constexpr float DEFAULT_DETECTION_RADIUS = 160.0f;
    constexpr float COLLIDER_WIDTH_RATIO = 0.7f;
    constexpr float COLLIDER_HEIGHT_RATIO = 0.45f;
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

    void setNavMap(const NavMap *navMap) { mNavMap = navMap; }
    const NavMap* getNavMap() const { return mNavMap; }

protected:
    void applySpriteRect(const Rectangle &spriteRect,
                         float desiredPixelHeight = EnemyConstants::DEFAULT_HEIGHT,
                         float colliderWidthRatio = EnemyConstants::COLLIDER_WIDTH_RATIO,
                         float colliderHeightRatio = EnemyConstants::COLLIDER_HEIGHT_RATIO);
    bool isPlayerWithinRange(Entity *player) const;

    float getMoveSpeed() const { return mMoveSpeed; }
    void setMoveSpeed(float speed) { mMoveSpeed = speed; }
    float getDetectionRadius() const { return mDetectionRadius; }
    void setDetectionRadius(float radius) { mDetectionRadius = radius; }

    virtual void updateBehaviour(float deltaTime, Entity *player) {}

private:
    float mMoveSpeed;
    float mDetectionRadius;
    const NavMap *mNavMap = nullptr;
};

#endif // ENEMY_H

