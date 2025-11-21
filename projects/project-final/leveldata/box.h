#ifndef BOX_H
#define BOX_H

#include "../lib/Entity.h"
#include "../lib/ResourceManager.h"

class Box : public Entity
{
public:
    explicit Box(Vector2 position);

    bool isCollected() const { return mCollected; }
    void markCollected();

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;

private:
    void configureSprite();
    bool mCollected = false;
};

#endif // BOX_H

