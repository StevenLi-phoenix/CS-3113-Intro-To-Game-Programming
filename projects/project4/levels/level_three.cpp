#include "level_three.h"

#include "../lib/helper.h"

const unsigned int LevelThree::LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1
};

LevelThree::LevelThree()
    : LevelBase(SceneID::LEVEL_THREE, SceneID::BOSS_FIGHT, "Skyreach Summit")
{
    mCameraZoom = 1.5f;
}

void LevelThree::setupEnemies()
{
    const Vector2 flyerScale {TILE_PIXEL_SIZE * 0.78f, TILE_PIXEL_SIZE * 0.72f};
    const Vector2 flyerAnchor = tileCenter(30, 4);
    Entity *skyPatrol = new Entity(flyerAnchor, flyerScale, "assets/slime/slime_green.png", NPC);
    configureSlimeSprite(skyPatrol);
    skyPatrol->setColliderDimensions({flyerScale.x * 0.55f, flyerScale.y * 0.55f});
    skyPatrol->setAcceleration({0.0f, 0.0f});
    skyPatrol->setSpeed(150);
    skyPatrol->setAIType(FLYER);
    skyPatrol->setAIState(WALKING);
    skyPatrol->setFlyParameters(flyerAnchor, TILE_PIXEL_SIZE * 4.0f, 150.0f, TILE_PIXEL_SIZE * 1.2f, 0.7f);
    registerEnemy(skyPatrol);

    const Vector2 guardianScale {TILE_PIXEL_SIZE * 0.85f, TILE_PIXEL_SIZE * 0.8f};
    const Vector2 guardianStart = tileCenter(34, 10);
    Entity *guardian = new Entity(guardianStart, guardianScale, "assets/slime/slime_green.png", NPC);
    configureSlimeSprite(guardian);
    guardian->setColliderDimensions({guardianScale.x * 0.55f, guardianScale.y * 0.65f});
    guardian->setAcceleration({0.0f, 981.0f});
    guardian->setSpeed(180);
    guardian->setAIType(FOLLOWER);
    guardian->setAIState(IDLE);
    guardian->setFollowRadius(300.0f);
    guardian->setFollowStopRadius(36.0f);
    guardian->setPatrolBounds(tileCenter(31, 10).x, tileCenter(36, 10).x);
    registerEnemy(guardian);
}

void LevelThree::onLevelCompleted()
{
    GameContext &ctx = GetGameContext();
    ctx.paused = false;
    RequestSceneChange(SceneID::BOSS_FIGHT);
}

void LevelThree::renderForeground()
{
    DrawFilledRectangle(mGoalWorldArea, ApplyAlpha(PURPLE, 0.35f));
    DrawRectangleBorder(mGoalWorldArea, 2.0f, PURPLE);
}
