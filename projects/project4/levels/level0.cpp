#include "level0.h"

#include <algorithm>

#include "../lib/helper.h"

namespace
{
    constexpr int TILESET_COLUMNS = 16;
    constexpr int TILESET_ROWS    = 16;
    constexpr const char *TILESET_PATH = "assets/world_tileset.png";
}

Level0::Level0() = default;

Level0::~Level0()
{
    shutdown();
}

void Level0::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#7EC0EE";
    mGameState.nextSceneID = 0;

    mMap = new Map(
        LEVEL0_WIDTH,
        LEVEL0_HEIGHT,
        (unsigned int *)mLevelData,
        TILESET_PATH,
        TILE_PIXEL_SIZE,
        TILESET_COLUMNS,
        TILESET_ROWS,
        mOrigin
    );
    mGameState.map = mMap;

    mWitch = new Witch();
    mGameState.xochitl = mWitch;

    if (mWitch && mMap)
    {
        mWitch->setAcceleration({0.0f, 981.0f});
        mWitch->setJumpingPower(575.0f);

        const float spawnColumn = 3.5f;
        const float spawnRow    = 5.5f;

        const float spawnX = mMap->getLeftBoundary() + spawnColumn * TILE_PIXEL_SIZE;
        const float spawnY = mMap->getTopBoundary()  + spawnRow    * TILE_PIXEL_SIZE;

        mWitch->setPosition({spawnX, spawnY});
    }

    mGameState.camera = {};
    mGameState.camera.offset = mOrigin;
    mGameState.camera.target = mWitch ? mWitch->getPosition() : mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.8f;
}

void Level0::handleInput()
{
    if (!mWitch) return;

    mWitch->beginInputFrame();

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  mWitch->moveLeft();
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) mWitch->moveRight();
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W)) mWitch->tryJump();

    mWitch->finalizeInputFrame();
}

void Level0::updateCamera()
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
        target.x = (mMap->getLeftBoundary() + mMap->getRightBoundary()) / 2.0f;
    }

    if (minY <= maxY)
    {
        target.y = std::max(minY, std::min(target.y, maxY));
    }
    else
    {
        target.y = (mMap->getTopBoundary() + mMap->getBottomBoundary()) / 2.0f;
    }

    mGameState.camera.target = target;
}

void Level0::update(float deltaTime)
{
    handleInput();

    if (mWitch)
    {
        mWitch->update(deltaTime, nullptr, mMap, nullptr, 0);
    }

    updateCamera();
}

void Level0::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    BeginMode2D(mGameState.camera);
    if (mMap)   mMap->render();
    if (mWitch) mWitch->render();
    EndMode2D();
}

void Level0::shutdown()
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
