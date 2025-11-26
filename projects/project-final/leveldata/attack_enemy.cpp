#include "attack_enemy.h"
#include "level1_consts.h"
#include "../lib/ResourceManager.h"
#include "../lib/Helper.h"

namespace
{
    constexpr float ATTACKER_HEIGHT = 52.0f;
    constexpr float SHOOT_INTERVAL = 2.6f;
    constexpr float SHOOT_VARIANCE = 0.65f;
    constexpr float SHOOT_RANGE = 620.0f;
    constexpr float SHOOT_STOP_RANGE = 520.0f;
    constexpr float MOVE_SPEED = 45.0f;
    constexpr float SPREAD_ANGLE = 0.25f; // radians between spread shots
}

AttackEnemy::AttackEnemy(Vector2 position,
                         int variant,
                         std::vector<SpreadProjectile> *projectilePool,
                         float desiredHeight)
    : Enemy(position, MOVE_SPEED, EnemyConstants::DEFAULT_DETECTION_RADIUS * 1.1f),
      mProjectiles(projectilePool),
      mVariant(std::clamp(variant, 0, 2)),
      mShootCooldown(0.5f),
      mShootInterval(SHOOT_INTERVAL)
{
    Rectangle spriteRect = resolveSpriteRect(mVariant);
    applySpriteRect(spriteRect, desiredHeight, EnemyConstants::COLLIDER_WIDTH_RATIO, EnemyConstants::COLLIDER_HEIGHT_RATIO);
    setTextureFacesLeft(false);
    setIsHorizontalFlipped(false);
}

Rectangle AttackEnemy::resolveSpriteRect(int variant) const
{
    const char *tags[] = { tags::ATTACK1, tags::ATTACK2, tags::ATTACK3 };
    const size_t tagCount = sizeof(tags) / sizeof(tags[0]);
    variant = std::clamp(variant, 0, static_cast<int>(tagCount) - 1);

    ResourceManager &rm = ResourceManager::instance();
    Rectangle rect = rm.getSpriteRect(tags[variant]);
    if (rect.width <= 0.0f || rect.height <= 0.0f)
    {
        return {0.0f, 0.0f, 24.0f, 24.0f};
    }
    return rect;
}

void AttackEnemy::updateBehaviour(float deltaTime,
                                  Entity *player,
                                  const std::vector<Entity*> &collidableEntities)
{
    (void)collidableEntities;
    setVelocity({0.0f, 0.0f});
    if (!player)
    {
        return;
    }

    Vector2 toPlayer = {
        player->getPosition().x - getPosition().x,
        player->getPosition().y - getPosition().y
    };
    const float distance = Vector2Length(toPlayer);
    if (distance > 0.0001f)
    {
        Vector2 dir = { toPlayer.x / distance, toPlayer.y / distance };
        setIsHorizontalFlipped(dir.x < 0.0f);

        if (distance > SHOOT_STOP_RANGE)
        {
            Vector2 move = { dir.x * MOVE_SPEED, dir.y * MOVE_SPEED };
            setVelocity(move);
        }
    }

    if (mShootCooldown > 0.0f)
    {
        mShootCooldown = std::max(0.0f, mShootCooldown - deltaTime);
    }

    if (distance <= SHOOT_RANGE && mShootCooldown <= 0.0f)
    {
        fireSpreadAt(player->getPosition());
        float variance = SHOOT_VARIANCE * (static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f);
        mShootCooldown = std::max(0.2f, mShootInterval - variance);
    }
}

void AttackEnemy::fireSpreadAt(const Vector2 &playerPos)
{
    if (!mProjectiles)
    {
        return;
    }
    Vector2 toPlayer = {
        playerPos.x - getPosition().x,
        playerPos.y - getPosition().y
    };
    const float baseAngle = atan2f(toPlayer.y, toPlayer.x);
    const float speeds[] = { -SPREAD_ANGLE, 0.0f, SPREAD_ANGLE };
    for (float offset : speeds)
    {
        SpreadProjectile proj;
        proj.position = getPosition();
        proj.speed = level1_consts::SPREAD_BALL_SPEED;
        proj.damage = level1_consts::SPREAD_BALL_DAMAGE;
        proj.radius = level1_consts::SPREAD_BALL_RADIUS;
        proj.lifetime = level1_consts::SPREAD_BALL_LIFETIME;
        const float angle = baseAngle + offset;
        proj.velocity = { cosf(angle), sinf(angle) };
        proj.angle = angle;
        mProjectiles->push_back(proj);
    }
}
