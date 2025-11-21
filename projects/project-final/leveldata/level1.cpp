#include "level1.h"
#include <cmath>

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    setChunkSize(mChunkSize);
    setChunkLoadRadius(mChunkLoadRadius);
    ensureTileTexture();
    if (!mPlayer)
    {
        mPlayer = new Player(c::ORIGIN, {32.0f, 32.0f});
        mPlayer->setIsActive(true);
    }
    if (mPlayer)
    {
        mCamera.target = mPlayer->getPosition();
    }
    updateChunkStream(true);
}

void Level1::update(float deltaTime)
{
    updateChunkStream();

    if (mPlayer)
    {
        mPlayer->update(deltaTime, nullptr, mMap);
        updateCameraTarget(mPlayer->getPosition(), deltaTime);
    }
}

void Level1::render()
{
    BeginMode2D(mCamera);
    if (mMap)
    {
        const double tRenderStart = GetTime();
        mMap->render();
        const double renderMs = (GetTime() - tRenderStart) * 1000.0;
        if (isDebugMode() && renderMs > 16.0)
        {
            LOG_INFO(TextFormat("Map render slow: %.2fms at chunkStart=(%d,%d)", renderMs, getChunkStartX(), getChunkStartY()));
        }
        else if (isDebugMode())
        {
            LOG_DEBUG(TextFormat("Map render: %.2fms at chunkStart=(%d,%d)", renderMs, getChunkStartX(), getChunkStartY()));
        }
    }

    if (mPlayer)
    {
        mPlayer->render();
    }
    if (isDebugMode())
    {
        drawChunkDebug();
        mPlayer->displayCollider();
    }
    EndMode2D();
    DrawFPS(10, 10);
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
    if (mTileTextureReady)
    {
        UnloadTexture(mTileTexture);
        mTileTextureReady = false;
    }
}

void Level1::buildProceduralMap()
{
    mMapGenerator = MapGenerator(mWorldSeed);

    mMapColumns = getLoadedColumns();
    mMapRows = getLoadedRows();

    const double t0 = GetTime();

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
    settings.startX = getChunkStartX();
    settings.startY = getChunkStartY();
    settings.zeroBias = 0.56f;

    mLevelData = std::move(mMapGenerator.generate(settings, mTileColumns * mTileRows));
    if (mLevelData.empty())
    {
        mLevelData.assign(mMapColumns * mMapRows, 1);
    }

    const float mapOriginX = (static_cast<float>(getChunkStartX()) + static_cast<float>(mMapColumns) / 2.0f) * mTileSize;
    const float mapOriginY = (static_cast<float>(getChunkStartY()) + static_cast<float>(mMapRows) / 2.0f) * mTileSize;
    const double tMapStart = GetTime();
    if (mMap)
    {
        mMap->refresh(mLevelData.data(), mMapColumns, mMapRows, {mapOriginX, mapOriginY});
        const double tMapEnd = GetTime();
        LOG_DEBUG(TextFormat("Map refresh (reuse) chunkStart=(%d,%d) took=%.2fms",
                             getChunkStartX(), getChunkStartY(),
                             (tMapEnd - tMapStart) * 1000.0));
    }
    else
    {
        mMap = new Map(
            mMapColumns,
            mMapRows,
            mLevelData.data(),
            mMapTexturePath,
            mTileSize,
            mTileColumns,
            mTileRows,
            {mapOriginX, mapOriginY},
            {}, // already pre-sliced texture
            mTileTextureReady ? &mTileTexture : nullptr
        );
        const double tMapEnd = GetTime();
        LOG_DEBUG(TextFormat("Map refresh (new map) chunkStart=(%d,%d) took=%.2fms",
                             getChunkStartX(), getChunkStartY(),
                             (tMapEnd - tMapStart) * 1000.0));
    }

    const double t1 = GetTime();
    LOG_DEBUG(TextFormat("Map rebuild chunkStart=(%d,%d) tiles=%dx%d took=%.2fms",
                         getChunkStartX(), getChunkStartY(), mMapColumns, mMapRows,
                         (t1 - t0) * 1000.0));
}

void Level1::updateChunkStream(bool forceRebuild)
{
    if (!mPlayer) return;

    const Vector2 position = mPlayer->getPosition();
    const bool changed = updateStreamChunk(position, mTileSize, forceRebuild);
    if (changed)
    {
        buildProceduralMap();
    }
}

void Level1::ensureTileTexture()
{
    if (mTileTextureReady) return;

    Image full = LoadImage(mMapTexturePath);
    Rectangle region = mTileAtlasRegion;
    // clamp region to texture bounds
    region.x = fmaxf(0.0f, fminf(region.x, static_cast<float>(full.width - 1)));
    region.y = fmaxf(0.0f, fminf(region.y, static_cast<float>(full.height - 1)));
    region.width = fmaxf(0.0f, fminf(region.width, static_cast<float>(full.width) - region.x));
    region.height = fmaxf(0.0f, fminf(region.height, static_cast<float>(full.height) - region.y));
    Image slice = ImageFromImage(full, region);
    mTileTexture = LoadTextureFromImage(slice);
    UnloadImage(slice);
    UnloadImage(full);
    mTileTextureReady = true;
}

void Level1::drawChunkDebug()
{
    const int span = getChunkSpan();
    const float chunkWorldSize = static_cast<float>(mChunkSize) * mTileSize;
    const float startX = static_cast<float>(getChunkStartX()) * mTileSize;
    const float startY = static_cast<float>(getChunkStartY()) * mTileSize;
    const Color border = {255, 0, 0, 140};

    for (int y = 0; y < span; ++y)
    {
        for (int x = 0; x < span; ++x)
        {
            Rectangle rect = {
                startX + static_cast<float>(x) * chunkWorldSize,
                startY + static_cast<float>(y) * chunkWorldSize,
                chunkWorldSize,
                chunkWorldSize
            };
            DrawRectangleLinesEx(rect, 2.0f, border);
        }
    }
}
