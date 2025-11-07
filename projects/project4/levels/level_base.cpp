#include "level_base.h"

#include <algorithm>
#include <map>
#include <vector>

#include "../lib/helper.h"

LevelBase::LevelBase(SceneID sceneID, SceneID nextSceneID, const char *levelName)
    : Scene(),
      mLevelName(levelName ? levelName : ""),
      mSceneID(sceneID),
      mNextSceneID(nextSceneID)
{
}

LevelBase::~LevelBase()
{
    shutdown();
}

void LevelBase::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#7EC0EE";
    mGameState.nextSceneID = 0;

    if (mMap)
    {
        delete mMap;
        mMap = nullptr;
    }

    const unsigned int *levelData = getLevelData();
    if (!levelData)
    {
        TraceLog(LOG_FATAL, "LevelBase: level data is null.");
        return;
    }

    mMap = new Map(
        getLevelWidth(),
        getLevelHeight(),
        const_cast<unsigned int *>(levelData),
        TILESET_PATH,
        TILE_PIXEL_SIZE,
        TILESET_COLUMNS,
        TILESET_ROWS,
        mOrigin
    );
    mGameState.map = mMap;

    const GameContext &ctx = GetGameContext();

    if (mWitch)
    {
        delete mWitch;
        mWitch = nullptr;
    }

    mWitch = new Witch(ctx.selectedVariant);
    mGameState.xochitl = mWitch;

    if (mWitch)
    {
        mWitch->setAcceleration({0.0f, 981.0f});
        mWitch->setJumpingPower(600.0f);
        mWitch->setSpeed(325);
    }

    mResetPending = false;
    mGameOverPending = false;
    mGameOverTimer = 0.0f;
    loadHitSound();
    loadDeathSound();
    loadJumpSound();
    clearEnemies();
    setupEnemies();

    updateGoalArea();
    respawnPlayer();

    mGameState.camera = {};
    mGameState.camera.offset = mOrigin;
    mGameState.camera.target = mWitch ? mWitch->getPosition() : mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = mCameraZoom;
}

void LevelBase::handleInput()
{
    GameContext &ctx = GetGameContext();

    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE))
    {
        ctx.paused = !ctx.paused;
    }

    if (!mWitch)
    {
        return;
    }

    mWitch->beginInputFrame();

    if (!ctx.paused)
    {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  mWitch->moveLeft();
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) mWitch->moveRight();
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        {
            const bool jumped = mWitch->tryJump();
            if (jumped && mJumpSoundLoaded)
            {
                PlaySound(mJumpSound);
            }
        }
    }

    mWitch->finalizeInputFrame();
}

void LevelBase::updateCamera()
{
    if (!mMap)
    {
        mGameState.camera.target = mOrigin;
        return;
    }

    Vector2 target = mWitch ? mWitch->getPosition() : mOrigin;

    const float zoom = (mGameState.camera.zoom <= 0.0f) ? 1.0f : mGameState.camera.zoom;
    const float halfViewWidth  = GetScreenWidth()  / (2.0f * zoom);
    const float halfViewHeight = GetScreenHeight() / (2.0f * zoom);

    const float minX = mMap->getLeftBoundary()   + halfViewWidth;
    const float maxX = mMap->getRightBoundary()  - halfViewWidth;
    const float minY = mMap->getTopBoundary()    + halfViewHeight;
    const float maxY = mMap->getBottomBoundary() - halfViewHeight;

    if (minX <= maxX)
    {
        target.x = std::max(minX, std::min(target.x, maxX));
    }
    else
    {
        target.x = (mMap->getLeftBoundary() + mMap->getRightBoundary()) * 0.5f;
    }

    if (minY <= maxY)
    {
        target.y = std::max(minY, std::min(target.y, maxY));
    }
    else
    {
        target.y = (mMap->getTopBoundary() + mMap->getBottomBoundary()) * 0.5f;
    }

    mGameState.camera.target = target;
}

void LevelBase::updateGoalArea()
{
    mGoalTileArea = getGoalTileArea();

    if (!mMap)
    {
        mGoalWorldArea = {0.0f, 0.0f, 0.0f, 0.0f};
        return;
    }

    const Vector2 topLeft   = tileToWorld({mGoalTileArea.x, mGoalTileArea.y});
    const float   width     = mGoalTileArea.width  * TILE_PIXEL_SIZE;
    const float   height    = mGoalTileArea.height * TILE_PIXEL_SIZE;

    mGoalWorldArea = {topLeft.x, topLeft.y, width, height};
}

void LevelBase::checkFallBoundary()
{
    if (!mWitch)
    {
        return;
    }

    float fallThreshold = static_cast<float>(GetScreenHeight());
    if (mMap)
    {
        fallThreshold = std::max(fallThreshold, mMap->getBottomBoundary() + TILE_PIXEL_SIZE * 0.5f);
    }

    if (mWitch->getPosition().y > fallThreshold)
    {
        GameContext &ctx = GetGameContext();
        ctx.lives -= 1;

        if (ctx.lives <= 0)
        {
            const float deathDuration = GAME_OVER_DELAY;
            if (mDeathSoundLoaded)
            {
                PlaySound(mDeathSound);
            }
            mGameOverPending = true;
            mGameOverTimer = deathDuration;
            mResetPending = true;
            ctx.paused = true;
        }
        else
        {
            respawnPlayer();
        }
    }
}

void LevelBase::respawnPlayer()
{
    if (!mWitch)
    {
        return;
    }

    const Vector2 spawnTile = getSpawnTile();
    mSpawnPoint = tileToWorld(spawnTile);

    mWitch->setPosition(mSpawnPoint);
    mWitch->setVelocity({0.0f, 0.0f});
    mWitch->setMovement({0.0f, 0.0f});
    mWitch->playIdle();
}

bool LevelBase::hasReachedGoal() const
{
    if (!mWitch)
    {
        return false;
    }

    if (mGoalWorldArea.width <= 0.0f || mGoalWorldArea.height <= 0.0f)
    {
        return false;
    }

    return PointInRectangle(mWitch->getPosition(), mGoalWorldArea);
}

Vector2 LevelBase::tileToWorld(const Vector2 &tile) const
{
    if (!mMap)
    {
        return mOrigin;
    }

    return {
        mMap->getLeftBoundary() + tile.x * TILE_PIXEL_SIZE,
        mMap->getTopBoundary()  + tile.y * TILE_PIXEL_SIZE
    };
}

void LevelBase::onLevelCompleted()
{
    GameContext &ctx = GetGameContext();
    ctx.currentLevelIndex += 1;
    ctx.paused = false;
    RequestSceneChange(mNextSceneID);
}

void LevelBase::renderForeground()
{
    // Default: no-op
}

void LevelBase::renderHUD() const
{
    const GameContext &ctx = GetGameContext();

    const int padding = 20;
    DrawText(TextFormat("Level: %s", mLevelName.c_str()), padding, padding, 28, DARKBLUE);
    DrawText(TextFormat("Lives: %d", ctx.lives), padding, padding + 34, 24, DARKBLUE);
    DrawText("Press P to pause", padding, padding + 62, 18, GRAY);

    if (ctx.paused)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ApplyAlpha(BLACK, 0.6f));
        const char *pausedText = "Paused";
        DrawText(pausedText, GetScreenWidth() / 2 - 120, GetScreenHeight() / 2 - 50, 48, RAYWHITE);
        const char *resumeText = "Press P or ESC to resume";
        DrawText(resumeText, GetScreenWidth() / 2 - 200,
                 GetScreenHeight() / 2 + 10, 24, LIGHTGRAY);
    }
}

void LevelBase::update(float deltaTime)
{
    handleInput();

    GameContext &ctx = GetGameContext();

    if (mGameOverPending)
    {
        mGameOverTimer -= deltaTime;
        if (mGameOverTimer <= 0.0f)
        {
            mGameOverPending = false;
            RequestSceneChange(SceneID::GAME_OVER);
        }
        return;
    }

    if (!ctx.paused)
    {
        if (mWitch)
        {
            mWitch->update(deltaTime, nullptr, mMap, nullptr, 0);
            checkFallBoundary();
        }

        updateEnemies(deltaTime);
        const bool playerHitEnemy = handlePlayerEnemyCollisions();

        if (!playerHitEnemy && hasReachedGoal())
        {
            onLevelCompleted();
        }
    }

    updateCamera();
}

void LevelBase::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    BeginMode2D(mGameState.camera);
    if (mMap)   mMap->render();
    renderEnemies();
    if (mWitch) mWitch->render();
    renderForeground();
    EndMode2D();

    renderHUD();
}

void LevelBase::shutdown()
{
    clearEnemies();
    unloadHitSound();
    unloadDeathSound();
    unloadJumpSound();
    mResetPending = false;

    if (mWitch)
    {
        delete mWitch;
        mWitch = nullptr;
    }

    if (mMap)
    {
        delete mMap;
        mMap = nullptr;
    }

    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}

void LevelBase::registerEnemy(Entity *enemy)
{
    if (!enemy) return;
    enemy->setEntityType(NPC);
    EnemyRecord record;
    record.entity = enemy;
    record.spawnPosition = enemy->getPosition();
    mEnemies.push_back(record);
}

void LevelBase::configureSlimeSprite(Entity *slime)
{
    if (!slime) return;

    static const std::map<Direction, std::vector<int>> SLIME_ANIM = []() {
        const std::vector<int> frames = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        std::map<Direction, std::vector<int>> animation;
        animation[RIGHT] = frames;
        animation[LEFT] = frames;
        animation[UP] = frames;
        animation[DOWN] = frames;
        return animation;
    }();

    slime->setTextureType(ATLAS);
    slime->setSpriteSheetDimensions({3.0f, 4.0f});
    slime->setAnimationAtlas(SLIME_ANIM);
    slime->setFrameSpeed(10);
    slime->setDirection(slime->getDirection());
}

void LevelBase::clearEnemies()
{
    for (EnemyRecord &record : mEnemies)
    {
        if (!record.entity) continue;
        delete record.entity;
        record.entity = nullptr;
    }
    mEnemies.clear();
}

void LevelBase::updateEnemies(float deltaTime)
{
    const float bottomLimit = mMap
        ? mMap->getBottomBoundary() + TILE_PIXEL_SIZE * 0.75f
        : static_cast<float>(GetScreenHeight());

    for (EnemyRecord &record : mEnemies)
    {
        Entity *enemy = record.entity;
        if (!enemy || !enemy->isActive()) continue;

        enemy->update(deltaTime, mWitch, mMap, nullptr, 0);

        if (enemy->getPosition().y > bottomLimit)
        {
            resetEnemyPosition(record);
        }
    }
}

void LevelBase::renderEnemies()
{
    for (const EnemyRecord &record : mEnemies)
    {
        Entity *enemy = record.entity;
        if (!enemy || !enemy->isActive()) continue;
        enemy->render();
    }
}

bool LevelBase::handlePlayerEnemyCollisions()
{
    if (!mWitch || mResetPending) return false;

    for (EnemyRecord &record : mEnemies)
    {
        Entity *enemy = record.entity;
        if (!enemy || !enemy->isActive()) continue;

        if (mWitch->intersects(*enemy))
        {
            if (onPlayerEnemyCollision(*enemy))
            {
                TraceLog(LOG_DEBUG, "[Debug] Collision resolved by scene override.");
                return true;
            }

            GameContext &ctx = GetGameContext();
            ctx.lives = std::max(0, ctx.lives - 1);
            ctx.paused = false;
            TraceLog(LOG_DEBUG, TextFormat("[Debug] Player hit by enemy. Lives remaining: %d", ctx.lives));

            if (mHitSoundLoaded)
            {
                PlaySound(mHitSound);
            }

            if (ctx.lives <= 0)
            {
                const float deathDuration = GAME_OVER_DELAY;
                if (mDeathSoundLoaded)
                {
                    PlaySound(mDeathSound);
                }
                mGameOverPending = true;
                mGameOverTimer = deathDuration;
                mResetPending = true;
                ctx.paused = true;
            }
            else
            {
                resetLevelState();
            }

            return true;
        }
    }

    return false;
}

Vector2 LevelBase::tileCenter(int tileX, int tileY) const
{
    return tileToWorld({static_cast<float>(tileX) + 0.5f, static_cast<float>(tileY) + 0.5f});
}

void LevelBase::resetEnemyPosition(EnemyRecord &record)
{
    if (!record.entity) return;
    record.entity->setPosition(record.spawnPosition);
    record.entity->setVelocity({0.0f, 0.0f});
    record.entity->setMovement({0.0f, 0.0f});
    record.entity->resetFlyAnchor();
    record.entity->activate();
}

void LevelBase::resetLevelState()
{
    GameContext &ctx = GetGameContext();
    mResetPending = true;
    mGameOverPending = false;
    mGameOverTimer = 0.0f;

    respawnPlayer();

    if (mWitch)
    {
        mGameState.camera.target = mWitch->getPosition();
    }

    for (EnemyRecord &record : mEnemies)
    {
        resetEnemyPosition(record);
    }

    ctx.paused = false;
    onLevelReset();

    mResetPending = false;
}

void LevelBase::loadHitSound()
{
    if (mHitSoundLoaded) return;

    if (!GetGameContext().audioReady)
    {
        return;
    }

    mHitSound = LoadSound("assets/sound/hurt.wav");
    if (mHitSound.frameCount > 0)
    {
        mHitSoundLoaded = true;
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load hurt sound: assets/sound/hurt.wav");
    }
}

void LevelBase::unloadHitSound()
{
    if (!mHitSoundLoaded) return;

    UnloadSound(mHitSound);
    mHitSoundLoaded = false;
}

void LevelBase::loadDeathSound()
{
    if (mDeathSoundLoaded) return;

    if (!GetGameContext().audioReady)
    {
        return;
    }

    mDeathSound = LoadSound("assets/sound/explosion.wav");
    if (mDeathSound.frameCount > 0)
    {
        mDeathSoundLoaded = true;
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load explosion sound: assets/sound/explosion.wav");
    }
}

void LevelBase::unloadDeathSound()
{
    if (!mDeathSoundLoaded) return;

    UnloadSound(mDeathSound);
    mDeathSoundLoaded = false;
}

void LevelBase::loadJumpSound()
{
    if (mJumpSoundLoaded) return;

    if (!GetGameContext().audioReady)
    {
        return;
    }

    mJumpSound = LoadSound("assets/sound/jump.wav");
    if (mJumpSound.frameCount > 0)
    {
        mJumpSoundLoaded = true;
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load jump sound: assets/sound/jump.wav");
    }
}

void LevelBase::unloadJumpSound()
{
    if (!mJumpSoundLoaded) return;

    UnloadSound(mJumpSound);
    mJumpSoundLoaded = false;
}

bool LevelBase::onPlayerEnemyCollision(Entity & /*enemy*/)
{
    return false;
}

void LevelBase::onLevelReset()
{
    // Default implementation: no-op
}

void LevelBase::setupEnemies()
{
    // Default implementation: no enemies.
}
