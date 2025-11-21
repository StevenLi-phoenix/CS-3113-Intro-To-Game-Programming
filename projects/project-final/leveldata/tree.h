#ifndef TREE_H
#define TREE_H

#include "../lib/Entity.h"
#include "../lib/ResourceManager.h"
#include "../constants.h"

namespace TreeConstants {
    constexpr float BASE_SCALE = 64.0f; // baseline sprite height from atlas
    constexpr float ROOT_COLLIDER_HEIGHT = 16.0f; // Only bottom portion has collision
    constexpr float ROOT_COLLIDER_WIDTH_RATIO = 0.25f; // Root is narrower than visual
    constexpr float MIN_SCALE = BASE_SCALE * 2.0f;   // minimum visual height in pixels
    constexpr float MAX_SCALE = BASE_SCALE * 4.0f;   // maximum visual height in pixels
    constexpr float MIN_ROOT_WIDTH_RATIO = 0.20f;
    constexpr float MAX_ROOT_WIDTH_RATIO = 0.35f;
}

class Tree : public Entity
{
private:
    float mTreeScale; // desired tree height in pixels
    int mTreeVariant; // Which tree sprite to use

public:
    Tree(Vector2 position,
         float treeScale = 1.0f,
         int treeVariant = 0,
         float rootColliderHeight = TreeConstants::ROOT_COLLIDER_HEIGHT,
         float rootColliderWidthRatio = TreeConstants::ROOT_COLLIDER_WIDTH_RATIO);
    ~Tree();

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    float getTreeScale() const { return mTreeScale; }
    int getTreeVariant() const { return mTreeVariant; }
};

#endif // TREE_H