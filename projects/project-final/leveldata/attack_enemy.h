#ifndef ATTACK_ENEMY_H
#define ATTACK_ENEMY_H

#include "../lib/Enemy.h"
#include "ResourceTags.h"
#include "spread_projectile.h"
#include <vector>

class AttackEnemy : public Enemy
{
public:
    AttackEnemy(Vector2 position,
                int variant,
                std::vector<SpreadProjectile> *projectilePool,
                float desiredHeight = EnemyConstants::DEFAULT_HEIGHT);

protected:
    void updateBehaviour(float deltaTime,
                         Entity *player,
                         const std::vector<Entity*> &collidableEntities) override;

private:
    Rectangle resolveSpriteRect(int variant) const;
    void fireSpreadAt(const Vector2 &playerPos);

    std::vector<SpreadProjectile> *mProjectiles;
    int mVariant;
    float mShootCooldown;
    float mShootInterval;
};

#endif // ATTACK_ENEMY_H
