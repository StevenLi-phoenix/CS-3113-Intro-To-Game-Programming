#ifndef BRANCH_H
#define BRANCH_H

#include "../lib/Entity.h"
#include "../lib/ResourceManager.h"
#include "../constants.h"

#include <algorithm>
#include <vector>

class Branch : public Entity
{
public:
    Branch(const Vector2 &start,
           const Vector2 &direction,
           float travelDistance,
           float speed = branch::THROW_SPEED,
           float damage = branch::PROJECTILE_DAMAGE,
           bool useShuriken = false);

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;

    void markSpent();
    bool isSpent() const { return mSpent; }
    bool isRecoverable() const { return mRecoverable; }
    void setRecoverable(bool value) { mRecoverable = value; }
    bool isCollected() const { return mCollected; }
    void markCollected();
    void setUseShuriken(bool value) { mUseShuriken = value; configureSprite(); }
    bool usesShuriken() const { return mUseShuriken; }
    float getDamage() const { return mDamage; }

private:
    void configureSprite();

    Vector2 mDirection;
    float mTravelDistance;
    float mSpeed;
    float mTraveled;
    float mDamage;
    bool mSpent;
    bool mRecoverable;
    bool mCollected;
    bool mUseShuriken;
};

#endif // BRANCH_H
