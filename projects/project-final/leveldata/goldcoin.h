#ifndef GOLDCOIN_H
#define GOLDCOIN_H

#include "../lib/Entity.h"
#include "../lib/ResourceManager.h"

class GoldCoin : public Entity
{
public:
    explicit GoldCoin(Vector2 position);

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;

private:
    void configureSprite();

    Vector2 mBasePosition;
    float mElapsed = 0.0f;
};

#endif // GOLDCOIN_H
