#ifndef SPREAD_PROJECTILE_H
#define SPREAD_PROJECTILE_H

#include "../lib/Helper.h"

struct SpreadProjectile
{
    Vector2 position{0.0f, 0.0f};
    Vector2 velocity{0.0f, 0.0f};
    float speed = 280.0f;
    float lifetime = 3.0f;
    float radius = 12.0f;
    float damage = 1.5f;
    float angle = 0.0f;
    int frame = 0;
    float frameTimer = 0.0f;
};

#endif // SPREAD_PROJECTILE_H
