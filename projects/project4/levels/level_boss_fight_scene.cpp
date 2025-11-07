#include "level_boss_fight_scene.h"

#include <algorithm>
#include <map>
#include <cmath>

#include "../lib/game_context.h"
#include "../lib/helper.h"

const unsigned int BossFightScene::LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
    2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

BossFightScene::BossFightScene()
    : LevelBase(SceneID::BOSS_FIGHT, SceneID::VICTORY, "Final Duel")
{
    mCameraZoom = 1.55f;
}

void BossFightScene::initialise()
{
    LevelBase::initialise();

    mBossHealth = BOSS_MAX_HEALTH;
    mInvulnerabilityTimer = 0.0f;
   mBossDefeated = false;
   mVictoryTimer = 0.0f;
   if (mWitch)
   {
       mLastPlayerPosition = mWitch->getPosition();
   }
    mBGColourHexCode = "#1B1525";
}

void BossFightScene::setupEnemies()
{
    const Vector2 bossScale {TILE_PIXEL_SIZE * 1.75f, TILE_PIXEL_SIZE * 1.75f};
    const Vector2 bossSpawn = tileCenter(28, 8);

    std::map<Direction, std::vector<int>> atlas = {
        {RIGHT, {0,1,2,3,4,5,6}},
        {LEFT,  {0,1,2,3,4,5,6}}
    };

    Entity *boss = new Entity(
        bossSpawn,
        bossScale,
        "assets/samurai/ATTACK 1.png",
        ATLAS,
        {1.0f, 7.0f},
        atlas,
        NPC
    );

    boss->setColliderDimensions({bossScale.x * 0.45f, bossScale.y * 0.85f});
    boss->setFrameSpeed(10);
    boss->setAcceleration({0.0f, 981.0f});
    boss->setAIType(FOLLOWER);
    boss->setAIState(WALKING);
    boss->setFollowRadius(900.0f);
    boss->setFollowStopRadius(32.0f);
    boss->setSpeed(220);
    boss->setJumpingPower(520.0f);

    const float leftBound = tileCenter(9, 8).x;
    const float rightBound = tileCenter(30, 8).x;
    boss->setPatrolBounds(leftBound, rightBound);

    registerEnemy(boss);
    mBoss = boss;
}

void BossFightScene::update(float deltaTime)
{
    if (mWitch)
    {
        mLastPlayerPosition = mWitch->getPosition();
    }

    if (mInvulnerabilityTimer > 0.0f)
    {
        mInvulnerabilityTimer -= deltaTime;
        if (mInvulnerabilityTimer < 0.0f) mInvulnerabilityTimer = 0.0f;
    }

    LevelBase::update(deltaTime);

    if (mWitch)
    {
        mLastPlayerPosition = mWitch->getPosition();
    }

    if (mBossDefeated)
    {
        if (mVictoryTimer > 0.0f)
        {
            mVictoryTimer -= deltaTime;
            if (mVictoryTimer <= 0.0f)
            {
                RequestSceneChange(SceneID::VICTORY);
                mVictoryTimer = 0.0f;
            }
        }
    }
}

bool BossFightScene::onPlayerEnemyCollision(Entity &enemy)
{
    if (!mWitch || &enemy != mBoss)
    {
        return false;
    }

    const float enemyTop = enemy.getPosition().y - (enemy.getColliderDimensions().y * 0.5f);
    const float playerCurrentTop = mWitch->getPosition().y - (mWitch->getColliderDimensions().y * 0.5f);
    const float playerCurrentBottom = mWitch->getPosition().y + (mWitch->getColliderDimensions().y * 0.5f);
    const float verticalVelocity = mWitch->getVelocity().y;

    const bool movingDownward = verticalVelocity > 40.0f;
    const bool playerCurrentlyAbove = playerCurrentTop <= (enemyTop + 14.0f);
    const bool overlappingEnemyHead = playerCurrentBottom >= (enemyTop - 12.0f);
    const float horizontalDistance = fabsf(mWitch->getPosition().x - enemy.getPosition().x);
    const float horizontalThreshold = (mWitch->getColliderDimensions().x + enemy.getColliderDimensions().x) * 0.5f + 20.0f;
    const bool horizontallyAligned = horizontalDistance <= horizontalThreshold;

    TraceLog(LOG_DEBUG, TextFormat(
        "[BossDebug] collision check: vy=%.2f currentTop=%.2f currentBottom=%.2f movingDown=%d aboveNow=%d overlapHead=%d horizAligned=%d invul=%.2f bossHP=%d",
        verticalVelocity,
        playerCurrentTop,
        playerCurrentBottom,
        movingDownward ? 1 : 0,
        playerCurrentlyAbove ? 1 : 0,
        overlappingEnemyHead ? 1 : 0,
        horizontallyAligned ? 1 : 0,
        mInvulnerabilityTimer,
        mBossHealth));

    if (movingDownward && playerCurrentlyAbove && overlappingEnemyHead && horizontallyAligned)
    {
        const float clampedY = enemyTop - (mWitch->getColliderDimensions().y * 0.5f) - 2.0f;
        mWitch->setPosition({mWitch->getPosition().x, clampedY});
        mWitch->setMovement({0.0f, 0.0f});

        Vector2 currentVelocity = mWitch->getVelocity();
        mWitch->setVelocity({currentVelocity.x, -600.0f});

        if (mInvulnerabilityTimer <= 0.0f && !mBossDefeated)
        {
            TraceLog(LOG_DEBUG, TextFormat("[BossDebug] Stomp registered. Boss HP before hit: %d", mBossHealth));
            mBossHealth = std::max(0, mBossHealth - 1);
            mInvulnerabilityTimer = 0.45f;

            if (mBossHealth <= 0)
            {
                enemy.deactivate();
                mBossDefeated = true;
                mVictoryTimer = VICTORY_DELAY;
                GameContext &ctx = GetGameContext();
                ctx.paused = false;
                TraceLog(LOG_DEBUG, "[BossDebug] Boss defeated!");
            }
        }
        else
        {
            TraceLog(LOG_DEBUG, TextFormat("[BossDebug] Stomp ignored due to invulnerability (timer=%.2f) or boss already defeated.", mInvulnerabilityTimer));
        }

        return true;
    }

    TraceLog(LOG_DEBUG, "[BossDebug] Stomp conditions not met, delegating to base collision handling.");
    mInvulnerabilityTimer = 0.0f;
    return false;
}

void BossFightScene::renderHUD() const
{
    LevelBase::renderHUD();

    const int padding = 20;
    const int infoY = padding + 96;
    DrawText(TextFormat("Boss HP: %d", std::max(mBossHealth, 0)), padding, infoY, 24, MAROON);

    const char *hint = mBossDefeated
        ? "Boss defeated! Preparing victory..."
        : "Tip: Jump on the boss to deal damage.";
    DrawText(hint, padding, infoY + 28, 20, LIGHTGRAY);
}

void BossFightScene::shutdown()
{
    LevelBase::shutdown();
    mBoss = nullptr;
}

void BossFightScene::onLevelReset()
{
    mInvulnerabilityTimer = 0.0f;
    if (mWitch)
    {
        mLastPlayerPosition = mWitch->getPosition();
    }
    if (!mBossDefeated)
    {
        mVictoryTimer = 0.0f;
    }
}
