#include "level1.h"
#include <cmath>
#include <algorithm>
#include "../lib/ResourceManager.h"

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    setChunkSize(mChunkSize);
    setChunkLoadRadius(mChunkLoadRadius);
    ensureTileTexture();
    ensureTreeAtlas();
    if (!mPlayer)
    {
        mPlayer = new Player(c::ORIGIN, {90.0f, 125.0f});
        mPlayer->setIsActive(true);
        mCollidableEntities.push_back(mPlayer);
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

    for (Entity* entity : mCollidableEntities)
    {
        entity->update(deltaTime, mPlayer, mMap, mCollidableEntities);
    }

    if (mPlayer)
    {
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

    // Sort entities by y position for proper occlusion (lower y renders first)
    std::sort(mCollidableEntities.begin(), mCollidableEntities.end(),
        [](const Entity* a, const Entity* b) {
            return a->getPosition().y < b->getPosition().y;
        });

    // Render all entities in sorted order
    for (Entity* entity : mCollidableEntities)
    {
        entity->render();
    }

    if (isDebugMode())
    {
        drawChunkDebug();
        for (Entity* entity : mCollidableEntities)
        {
            entity->displayCollider();
        }
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

    for (Tree* tree : mTrees)
    {
        delete tree;
    }
    mTrees.clear();
    mCollidableEntities.clear();

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
}

void Level1::generateTrees()
{
    // Remove old trees from collidable entities
    for (Tree* tree : mTrees)
    {
        auto it = std::find(mCollidableEntities.begin(), mCollidableEntities.end(), tree);
        if (it != mCollidableEntities.end())
        {
            mCollidableEntities.erase(it);
        }
        delete tree;
    }
    mTrees.clear();

    const double tGenStart = GetTime();

    struct TreeSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.995f;
        int spacing = 1;
        float minHeightPx = TreeConstants::MIN_SCALE;
        float maxHeightPx = TreeConstants::MAX_SCALE;
        float baseRootHeight = TreeConstants::ROOT_COLLIDER_HEIGHT;
        float minRootWidthRatio = TreeConstants::MIN_ROOT_WIDTH_RATIO;
        float maxRootWidthRatio = TreeConstants::MAX_ROOT_WIDTH_RATIO;
    };

    const TreeSpawnSettings spawnSettings{
        0x3fa8bc91u,
        0.9945f,
        1,
        TreeConstants::MIN_SCALE,
        TreeConstants::MAX_SCALE,
        TreeConstants::ROOT_COLLIDER_HEIGHT,
        TreeConstants::MIN_ROOT_WIDTH_RATIO,
        TreeConstants::MAX_ROOT_WIDTH_RATIO
    };

    const int startX = getChunkStartX();
    const int startY = getChunkStartY();

    const int spacing = std::max(spawnSettings.spacing, 1);

    // Generate trees based on world coordinates using deterministic noise
    for (int row = 0; row < mMapRows; row += spacing)
    {
        for (int col = 0; col < mMapColumns; col += spacing)
        {
            const int worldX = startX + col;
            const int worldY = startY + row;

            float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt);

            if (spawnNoise < spawnSettings.spawnThreshold)
            {
                continue;
            }

            const float variantNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt + 1u);
            const float scaleNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt + 2u);
            const float rootWidthNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt + 3u);
            const float rootHeightNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt + 4u);

            int treeVariant = static_cast<int>(variantNoise * static_cast<float>(c::TREE_VARIANT_COUNT));
            treeVariant = std::clamp(treeVariant, 0, c::TREE_VARIANT_COUNT - 1);

            const float treeHeightPx = spawnSettings.minHeightPx +
                                       scaleNoise * (spawnSettings.maxHeightPx - spawnSettings.minHeightPx);

            const float rootWidthRatio = spawnSettings.minRootWidthRatio +
                                         rootWidthNoise * (spawnSettings.maxRootWidthRatio - spawnSettings.minRootWidthRatio);

            const float rootHeight = spawnSettings.baseRootHeight *
                                     (0.75f + rootHeightNoise * 0.5f);

            float worldPosX = (static_cast<float>(worldX) + 0.5f) * mTileSize;
            float worldPosY = (static_cast<float>(worldY) + 0.5f) * mTileSize;

            Tree* tree = new Tree({worldPosX, worldPosY},
                                  treeHeightPx,
                                  treeVariant,
                                  rootHeight,
                                  rootWidthRatio);
            mTrees.push_back(tree);
            mCollidableEntities.push_back(tree);
        }
    }

    const double tGenEnd = GetTime();
    LOG_DEBUG(TextFormat("Generated %d trees in %.2fms",
                         static_cast<int>(mTrees.size()),
                         (tGenEnd - tGenStart) * 1000.0));
}

void Level1::updateChunkStream(bool forceRebuild)
{
    if (!mPlayer) return;

    const Vector2 position = mPlayer->getPosition();
    const bool changed = updateStreamChunk(position, mTileSize, forceRebuild);
    if (changed)
    {
        buildProceduralMap();
        generateTrees();
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

void Level1::ensureTreeAtlas()
{
    ResourceManager &rm = ResourceManager::instance();
    if (rm.hasTexture(ResourceKeys::WORLD_ATLAS))
    {
        return;
    }

    if (!rm.loadAtlas(ResourceKeys::WORLD_ATLAS, mMapTexturePath, mMapAtlasMetadataPath))
    {
        LOG_ERROR(TextFormat("Failed to load world atlas texture (%s) or metadata (%s)",
                             mMapTexturePath, mMapAtlasMetadataPath));
    }
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
