#include "tree.h"
#include <algorithm>
#include <vector>

Tree::Tree(Vector2 position,
           float treeScale,
           int treeVariant,
           float rootColliderHeight,
           float rootColliderWidthRatio,
           float health)
    : Enemy(position, 0.0f, 0.0f),
      mTreeScale(treeScale),
      mTreeVariant(0)
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *texture = rm.getTexture(ResourceKeys::WORLD_ATLAS);

    Rectangle spriteRect = {0.0f, 0.0f, TreeConstants::BASE_SCALE * 0.5f, TreeConstants::BASE_SCALE};
    const std::vector<Rectangle> &treeRects = rm.getSpriteRects(TreeConstants::SPRITE_TAG);
    const size_t rectCount = treeRects.size();
    if (rectCount > 0)
    {
        const int countAsInt = static_cast<int>(rectCount);
        int normalizedVariant = treeVariant % countAsInt;
        if (normalizedVariant < 0) normalizedVariant += countAsInt;
        mTreeVariant = normalizedVariant;
        spriteRect = treeRects[mTreeVariant];
    }

    if (texture)
    {
        setTexture(*texture);
        setOwnsTexture(false);
        setCustomSourceRect(spriteRect);
    }

    const float spriteHeight = std::max(spriteRect.height, 1.0f);
    const float desiredPixelHeight = std::max(treeScale, TreeConstants::BASE_SCALE);
    const float scaleFactor = desiredPixelHeight / spriteHeight;
    mTreeScale = desiredPixelHeight;

    Vector2 visualScale = {
        spriteRect.width * scaleFactor,
        spriteRect.height * scaleFactor
    };
    setScale(visualScale);

    const float appliedRootHeight = std::max(rootColliderHeight * scaleFactor, 1.0f);
    const float appliedRootWidthRatio = std::max(rootColliderWidthRatio, 0.05f);
    Vector2 rootColliderSize = {
        visualScale.x * appliedRootWidthRatio,
        appliedRootHeight
    };
    setColliderDimensions(rootColliderSize);

    setPosition(position);
    const float spriteOffsetY = (appliedRootHeight - visualScale.y) / 2.0f;
    setTextureOffset({0.0f, spriteOffsetY});

    setMaxHealth(std::max(health, 1.0f));
    setHealth(std::max(health, 1.0f));
    setCanCollide(true);
    setIsActive(true);
    setIsPushable(false);
}

void Tree::update(float deltaTime,
                  Entity *player,
                  Map *map,
                  const std::vector<Entity*> &collidableEntities)
{
    setVelocity({0.0f, 0.0f});
    setMovement({0.0f, 0.0f});
    Enemy::update(deltaTime, player, map, collidableEntities);
}
