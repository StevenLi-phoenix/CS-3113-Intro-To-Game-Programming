#include "level_two.h"

#include "../lib/helper.h"

const unsigned int LevelTwo::LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1
};

LevelTwo::LevelTwo()
    : LevelBase(SceneID::LEVEL_TWO, SceneID::LEVEL_THREE, "Luminous Ruins")
{
    mCameraZoom = 1.55f;
}

void LevelTwo::setupEnemies()
{
    const Vector2 followerScale {TILE_PIXEL_SIZE * 0.85f, TILE_PIXEL_SIZE * 0.8f};
    const Vector2 followerStart = tileCenter(21, 8);
    Entity *pursuer = new Entity(followerStart, followerScale, "assets/slime/slime_green.png", NPC);
    configureSlimeSprite(pursuer);
    pursuer->setColliderDimensions({followerScale.x * 0.55f, followerScale.y * 0.65f});
    pursuer->setAcceleration({0.0f, 981.0f});
    pursuer->setSpeed(175);
    pursuer->setAIType(FOLLOWER);
    pursuer->setAIState(IDLE);
    pursuer->setFollowRadius(320.0f);
    pursuer->setFollowStopRadius(38.0f);
    const float patrolLeft = tileCenter(18, 8).x;
    const float patrolRight = tileCenter(24, 8).x;
    pursuer->setPatrolBounds(patrolLeft, patrolRight);
    registerEnemy(pursuer);
}

void LevelTwo::renderForeground()
{
    DrawFilledRectangle(mGoalWorldArea, ApplyAlpha(ORANGE, 0.3f));
    DrawRectangleBorder(mGoalWorldArea, 2.0f, ORANGE);
}
