#include "level1.h"

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    buildProceduralMap();
    if (!mPlayer)
    {
        mPlayer = new Player(c::ORIGIN, {32.0f, 32.0f});
        mPlayer->setIsActive(true);
    }
    if (mPlayer)
    {
        mGameState.camera.target = mPlayer->getPosition();
    }
}

void Level1::update(float deltaTime)
{
    if (mPlayer)
    {
        mPlayer->update(deltaTime, nullptr, mMap);
        updateCameraTarget(mPlayer->getPosition(), deltaTime);
    }
}

void Level1::render()
{
    BeginMode2D(mGameState.camera);
    if (mMap)
    {
        mMap->render();
    }

    if (mPlayer)
    {
        mPlayer->render();
        mPlayer->displayCollider();
    }
    EndMode2D();
    DrawText("Level 1 - WIP", c::SCREEN_WIDTH / 2 - 100, c::SCREEN_HEIGHT / 2, 24, DARKBLUE);
}

void Level1::shutdown()
{
    LOG_INFO("Level1 shutdown");
    delete mPlayer;
    mPlayer = nullptr;
    delete mMap;
    mMap = nullptr;
    mLevelData.clear();
}

void Level1::buildProceduralMap()
{
    mMapGenerator = MapGenerator(mWorldSeed);

    MapGenerator::GenerationSettings settings;
    settings.columns = mMapColumns;
    settings.rows = mMapRows;
    settings.discreteRandom = true;
    settings.discreteSalt = mNoiseSalt;
    settings.scale = 18.0f;
    settings.octaves = 5;
    settings.persistence = 0.55f;
    settings.lacunarity = 2.05f;
    settings.perlinWeight = 0.6f;
    settings.simplexWeight = 0.4f;
    settings.offsetX = 0.0f;
    settings.offsetY = 32.0f;
    settings.zeroBias = 0.56f;

    mLevelData = mMapGenerator.generate(settings, mTileColumns * mTileRows);
    if (mLevelData.empty())
    {
        mLevelData.assign(mMapColumns * mMapRows, 1);
    }

    delete mMap;
    mMap = new Map(
        mMapColumns,
        mMapRows,
        mLevelData.data(),
        mMapTexturePath,
        mTileSize,
        mTileColumns,
        mTileRows,
        c::ORIGIN,
        mTileAtlasRegion
    );

    mGameState.map = mMap;
}
