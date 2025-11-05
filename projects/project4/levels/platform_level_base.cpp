#include "platform_level_base.h"

#include <algorithm>

#include "../lib/helper.h"

PlatformLevelBase::PlatformLevelBase(SceneID sceneID, SceneID nextSceneID, const char *levelName)
    : Scene(),
      mLevelName(levelName ? levelName : ""),
      mSceneID(sceneID),
      mNextSceneID(nextSceneID)
{
}

PlatformLevelBase::~PlatformLevelBase()
{
    shutdown();
}

void PlatformLevelBase::initialise()
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
        TraceLog(LOG_FATAL, "PlatformLevelBase: level data is null.");
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

    updateGoalArea();
    respawnPlayer();

    mGameState.camera = {};
    mGameState.camera.offset = mOrigin;
    mGameState.camera.target = mWitch ? mWitch->getPosition() : mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = mCameraZoom;
}

void PlatformLevelBase::handleInput()
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
            mWitch->tryJump();
        }
    }

    mWitch->finalizeInputFrame();
}

void PlatformLevelBase::updateCamera()
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

void PlatformLevelBase::updateGoalArea()
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

void PlatformLevelBase::checkFallBoundary()
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
            RequestSceneChange(SceneID::GAME_OVER);
        }
        else
        {
            respawnPlayer();
        }
    }
}

void PlatformLevelBase::respawnPlayer()
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

bool PlatformLevelBase::hasReachedGoal() const
{
    if (!mWitch)
    {
        return false;
    }

    if (mGoalWorldArea.width <= 0.0f || mGoalWorldArea.height <= 0.0f)
    {
        return false;
    }

    return CheckCollisionPointRec(mWitch->getPosition(), mGoalWorldArea);
}

Vector2 PlatformLevelBase::tileToWorld(const Vector2 &tile) const
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

void PlatformLevelBase::onLevelCompleted()
{
    GameContext &ctx = GetGameContext();
    ctx.currentLevelIndex += 1;
    ctx.paused = false;
    RequestSceneChange(mNextSceneID);
}

void PlatformLevelBase::renderForeground()
{
    // Default: no-op
}

void PlatformLevelBase::renderHUD() const
{
    const GameContext &ctx = GetGameContext();

    const int padding = 20;
    DrawText(TextFormat("Level: %s", mLevelName.c_str()), padding, padding, 28, DARKBLUE);
    DrawText(TextFormat("Lives: %d", ctx.lives), padding, padding + 34, 24, DARKBLUE);
    DrawText("Press P to pause", padding, padding + 62, 18, GRAY);

    if (ctx.paused)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        const char *pausedText = "Paused";
        const int textWidth = MeasureText(pausedText, 48);
        DrawText(pausedText, (GetScreenWidth() - textWidth) / 2, GetScreenHeight() / 2 - 50, 48, RAYWHITE);
        DrawText("Press P or ESC to resume", (GetScreenWidth() - MeasureText("Press P or ESC to resume", 24)) / 2,
                 GetScreenHeight() / 2 + 10, 24, LIGHTGRAY);
    }
}

void PlatformLevelBase::update(float deltaTime)
{
    handleInput();

    GameContext &ctx = GetGameContext();

    if (!ctx.paused && mWitch)
    {
        mWitch->update(deltaTime, nullptr, mMap, nullptr, 0);
        checkFallBoundary();

        if (hasReachedGoal())
        {
            onLevelCompleted();
        }
    }

    updateCamera();
}

void PlatformLevelBase::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    BeginMode2D(mGameState.camera);
    if (mMap)   mMap->render();
    if (mWitch) mWitch->render();
    renderForeground();
    EndMode2D();

    renderHUD();
}

void PlatformLevelBase::shutdown()
{
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
