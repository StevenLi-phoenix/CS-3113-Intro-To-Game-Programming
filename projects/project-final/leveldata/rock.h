#ifndef ROCK_H
#define ROCK_H

#include "../lib/Entity.h"

namespace RockConstants {
    constexpr float DEFAULT_COLLIDER_HEIGHT_RATIO = 0.6f;
    constexpr float DEFAULT_COLLIDER_WIDTH_RATIO = 0.8f;
}

class Rock : public Entity
{
public:
    Rock(Vector2 position,
         const Rectangle &spriteRect,
         float targetHeight,
         float colliderHeightRatio = RockConstants::DEFAULT_COLLIDER_HEIGHT_RATIO,
         float colliderWidthRatio = RockConstants::DEFAULT_COLLIDER_WIDTH_RATIO);
    ~Rock() = default;

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;
};

#endif // ROCK_H


