#include "level_one.h"

#include "../lib/helper.h"

const unsigned int LevelOne::LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,2,2,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1
};

LevelOne::LevelOne()
    : LevelBase(SceneID::LEVEL_ONE, SceneID::LEVEL_TWO, "Windy Meadows")
{
    mCameraZoom = 1.7f;
}

void LevelOne::setupEnemies()
{
    const Vector2 enemyScale {TILE_PIXEL_SIZE * 0.8f, TILE_PIXEL_SIZE * 0.7f};
    const Vector2 wanderStart = tileCenter(8, 7);
    Entity *wanderer = new Entity(wanderStart, enemyScale, "assets/slime/slime_green.png", NPC);
    configureSlimeSprite(wanderer);
    wanderer->setColliderDimensions({enemyScale.x * 0.6f, enemyScale.y * 0.7f});
    wanderer->setAcceleration({0.0f, 981.0f});
    wanderer->setSpeed(140);
    wanderer->setAIType(WANDERER);
    wanderer->setAIState(WALKING);
    wanderer->setWanderDirection(-1);
    const float leftBound = tileCenter(7, 7).x;
    const float rightBound = tileCenter(12, 7).x;
    wanderer->setPatrolBounds(leftBound, rightBound);
    registerEnemy(wanderer);
}

void LevelOne::renderForeground()
{
    DrawFilledRectangle(mGoalWorldArea, ApplyAlpha(SKYBLUE, 0.3f));
    DrawRectangleBorder(mGoalWorldArea, 2.0f, SKYBLUE);
}
