#ifndef DOG_H
#define DOG_H

#include "../lib/Enemy.h"
#include <vector>

namespace DogConstants
{
    constexpr int VARIANT_COUNT = 4;
    constexpr float DEFAULT_HEIGHT = 28.0f;
    constexpr float MIN_HEIGHT = 24.0f;
    constexpr float MAX_HEIGHT = 36.0f;
    constexpr float CHASE_SPEED = 45.0f;
    constexpr float PATROL_SPEED = 10.0f;
    constexpr float DETECTION_RADIUS = 140.0f;
    constexpr float COLLIDER_WIDTH_RATIO = 0.8f;
    constexpr float COLLIDER_HEIGHT_RATIO = 0.5f;
    constexpr float PATH_REFRESH_INTERVAL = 0.35f;
    constexpr float PATH_NODE_REACHED_RADIUS = 12.0f;
    constexpr const char *SPRITE_TAGS[VARIANT_COUNT] = {"DOG1", "DOG2", "DOG3", "DOG4"};
}

class Dog : public Enemy
{
public:
    Dog(Vector2 position,
        int variant = 0,
        float desiredHeightPixels = DogConstants::DEFAULT_HEIGHT);
protected:
    void updateBehaviour(float deltaTime, Entity *player) override;

private:
    Rectangle resolveSpriteRect(int variant) const;

    int mVariant;
    float mDesiredHeight;
    bool mIsChasing = false;
    bool mHasPath = false;
    std::vector<Vector2> mCurrentPath;
    size_t mCurrentPathIndex = 1;
    float mPathCooldown = 0.0f;
};

#endif // DOG_H

