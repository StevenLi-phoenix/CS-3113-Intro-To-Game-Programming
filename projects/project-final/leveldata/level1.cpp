#include "level1.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include "../lib/ResourceManager.h"

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    setChunkSize(mChunkSize);
    setChunkLoadRadius(mChunkLoadRadius);
    ensureTreeAtlas();
    ensureTileTexture();
    if (!mPlayer)
    {
        mPlayer = new Player(c::ORIGIN, {54.0f, 75.0f});
        mPlayer->setIsActive(true);
        mCollidableEntities.push_back(mPlayer);
    }
    if (mPlayer)
    {
        mCamera.target = mPlayer->getPosition();
    }
    ensureMusicNotes();
    updateChunkStream(true);
}

void Level1::update(float deltaTime)
{
    updateChunkStream();
    ensureMusicNotes();
    updateMusicNoteBehaviour();

    for (Entity* entity : mCollidableEntities)
    {
        entity->update(deltaTime, mPlayer, mMap, mCollidableEntities);
    }

    updatePlayerAttack(deltaTime);

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
            // TODO: remove this
            // LOG_DEBUG(TextFormat("Map render: %.2fms at chunkStart=(%d,%d)", renderMs, getChunkStartX(), getChunkStartY()));
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

    for (Enemy* enemy : mEnemies)
    {
        delete enemy;
    }
    mEnemies.clear();

    if (!mMusicNotes.empty())
    {
        for (MusicNote* note : mMusicNotes)
        {
            auto it = std::find(mCollidableEntities.begin(), mCollidableEntities.end(), note);
            if (it != mCollidableEntities.end())
            {
                mCollidableEntities.erase(it);
            }
            delete note;
        }
        mMusicNotes.clear();
    }

    mCollidableEntities.clear();

    mLevelData.clear();
    if (mTileTextureReady)
    {
        mTileTexture = nullptr;
        mTileTextureReady = false;
    }
}

void Level1::ensureMusicNotes()
{
    if (!mPlayer)
    {
        return;
    }

    const size_t targetCount = static_cast<size_t>(std::max(0, mNoteCount));
    if (mMusicNotes.size() >= targetCount)
    {
        return;
    }

    while (mMusicNotes.size() < targetCount)
    {
        const size_t slotIndex = mMusicNotes.size();
        const MusicNote::Variant variant = mNoteSequence[slotIndex % mNoteSequence.size()];

        MusicNote::FollowConfig followConfig;
        followConfig.lagCoefficient = combat::NOTE_FOLLOW_LAG;
        followConfig.lerpSpeed = combat::NOTE_FOLLOW_LERP;
        followConfig.bobAmplitude = combat::NOTE_BOB_AMPLITUDE;
        followConfig.bobSpeed = combat::NOTE_BOB_SPEED;

        MusicNote *note = new MusicNote(variant, mPlayer, followConfig);
        note->setParent(mPlayer);
        note->setCanCollide(false);
        note->setIsActive(true);
        mMusicNotes.push_back(note);
        mCollidableEntities.push_back(note);
    }

    refreshMusicNoteSlots();
}

void Level1::refreshMusicNoteSlots()
{
    const size_t total = mMusicNotes.size();
    if (total == 0)
    {
        return;
    }

    for (size_t i = 0; i < total; ++i)
    {
        if (mMusicNotes[i])
        {
            mMusicNotes[i]->setOrbitSlot(i, total);
        }
    }
}

void Level1::updateMusicNoteBehaviour()
{
    const bool enemyNearby = hasEnemyWithinRadius(mNoteIdleRadius);
    for (MusicNote* note : mMusicNotes)
    {
        if (!note)
        {
            continue;
        }
        note->setOrbitSuppressed(enemyNearby);
    }
}

MusicNote::Variant Level1::currentNoteVariant() const
{
    return mNoteSequence[mNoteSequenceIndex % mNoteSequence.size()];
}

void Level1::advanceNoteVariant()
{
    mNoteSequenceIndex = (mNoteSequenceIndex + 1) % mNoteSequence.size();
}

Enemy* Level1::findNearestEnemy(float maxRange) const
{
    if (!mPlayer)
    {
        return nullptr;
    }

    const float limitSq = maxRange > 0.0f ? maxRange * maxRange : std::numeric_limits<float>::max();
    Enemy* closest = nullptr;
    float bestDistanceSq = limitSq;

    const Vector2 playerPos = mPlayer->getPosition();
    for (Enemy* enemy : mEnemies)
    {
        if (!enemy || !enemy->getIsActive())
        {
            continue;
        }

        Vector2 toEnemy = {
            enemy->getPosition().x - playerPos.x,
            enemy->getPosition().y - playerPos.y
        };
        const float distanceSq = toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y;

        if (distanceSq <= bestDistanceSq)
        {
            bestDistanceSq = distanceSq;
            closest = enemy;
        }
    }

    return closest;
}

bool Level1::hasEnemyWithinRadius(float radius) const
{
    return findNearestEnemy(radius) != nullptr;
}

MusicNote* Level1::findAvailableNoteForVariant(MusicNote::Variant variant)
{
    for (MusicNote* note : mMusicNotes)
    {
        if (note && note->getVariant() == variant && note->isAvailableForAttack())
        {
            return note;
        }
    }
    return nullptr;
}

void Level1::updatePlayerAttack(float deltaTime)
{
    if (!mPlayer || mMusicNotes.empty())
    {
        return;
    }

    mAttackTimer -= deltaTime;
    if (mAttackTimer > 0.0f)
    {
        return;
    }

    Enemy* target = findNearestEnemy(mAttackRange);
    if (!target)
    {
        mAttackTimer = 0.0f;
        return;
    }

    const MusicNote::Variant variant = currentNoteVariant();
    MusicNote* note = findAvailableNoteForVariant(variant);
    if (!note)
    {
        // wait briefly for the correct note to return to formation
        mAttackTimer = 0.05f;
        return;
    }

    const int damage = MusicNote::damageForVariant(variant);
    const bool defeated = target->applyDamage(static_cast<float>(damage));
    if (defeated && isDebugMode())
    {
        LOG_DEBUG(TextFormat("Music attack defeated enemy[%p] damage=%d", target, damage));
    }

    note->launchAttack(target->getPosition());
    advanceNoteVariant();
    mAttackTimer = std::max(mAttackIntervalSeconds, 0.01f);
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

    const double tNavStart = GetTime();
    mNavMap.build(mLevelData.data(),
                  mMapColumns,
                  mMapRows,
                  mTileSize,
                  getChunkStartX(),
                  getChunkStartY());
    const double tNavEnd = GetTime();
    if (isDebugMode())
    {
        LOG_DEBUG(TextFormat("NavMap build chunkStart=(%d,%d) size=%dx%d took=%.2fms",
                             getChunkStartX(),
                             getChunkStartY(),
                             mMapColumns,
                             mMapRows,
                             (tNavEnd - tNavStart) * 1000.0));
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
            mTileAtlasRegion,
            mTileTextureReady ? mTileTexture : nullptr
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

    ResourceManager &rm = ResourceManager::instance();
    const std::vector<Rectangle> &treeRects = rm.getSpriteRects(TreeConstants::SPRITE_TAG);
    const int treeVariantCount = std::max(1, static_cast<int>(treeRects.size()));
    if (treeRects.empty())
    {
        LOG_WARNING("Tree sprites missing tag 'TREE' in atlas metadata; using fallback variant.");
    }

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

            int treeVariant = static_cast<int>(variantNoise * static_cast<float>(treeVariantCount));
            treeVariant = std::clamp(treeVariant, 0, treeVariantCount - 1);

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

void Level1::generateEnemies()
{
    for (Enemy* enemy : mEnemies)
    {
        auto it = std::find(mCollidableEntities.begin(), mCollidableEntities.end(), enemy);
        if (it != mCollidableEntities.end())
        {
            mCollidableEntities.erase(it);
        }
        delete enemy;
    }
    mEnemies.clear();

    struct EnemySpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.80f;
        int spacing = 4;
        float minHeightPx = DogConstants::MIN_HEIGHT;
        float maxHeightPx = DogConstants::MAX_HEIGHT;
        float maxSpawnDistance = 0.0f;
    };

    const float chunkWorldSize = static_cast<float>(mChunkSize) * mTileSize;
    const EnemySpawnSettings spawnSettings{
        0x5f3759d5u,
        0.3f,
        4,
        DogConstants::MIN_HEIGHT,
        DogConstants::MAX_HEIGHT,
        chunkWorldSize * 0.85f
    };

    const int startX = getChunkStartX();
    const int startY = getChunkStartY();
    const int spacing = std::max(spawnSettings.spacing, 1);

    const double tGenStart = GetTime();
    Vector2 playerPos = mPlayer ? mPlayer->getPosition()
                                : Vector2{
                                      (static_cast<float>(startX) + static_cast<float>(mMapColumns) * 0.5f) * mTileSize,
                                      (static_cast<float>(startY) + static_cast<float>(mMapRows) * 0.5f) * mTileSize
                                  };
    const float maxSpawnDistanceSq = spawnSettings.maxSpawnDistance * spawnSettings.maxSpawnDistance;
    int spawnedCount = 0;

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
            const float heightNoise = mMapGenerator.whiteNoise(worldX, worldY, spawnSettings.salt + 2u);

            int variant = static_cast<int>(variantNoise * static_cast<float>(DogConstants::VARIANT_COUNT));
            variant = std::clamp(variant, 0, DogConstants::VARIANT_COUNT - 1);

            const float dogHeight = spawnSettings.minHeightPx +
                                    heightNoise * (spawnSettings.maxHeightPx - spawnSettings.minHeightPx);

            float worldPosX = (static_cast<float>(worldX) + 0.5f) * mTileSize;
            float worldPosY = (static_cast<float>(worldY) + 0.5f) * mTileSize;

            const float dx = worldPosX - playerPos.x;
            const float dy = worldPosY - playerPos.y;
            if ((dx * dx + dy * dy) > maxSpawnDistanceSq)
            {
                continue;
            }

            Dog* dog = new Dog({worldPosX, worldPosY}, variant, dogHeight);
            dog->setNavMap(&mNavMap);
            mEnemies.push_back(dog);
            mCollidableEntities.push_back(dog);
            spawnedCount++;
            if (isDebugMode())
            {
                const float dist = sqrtf((dx * dx) + (dy * dy));
                LOG_DEBUG(TextFormat("Enemy spawn[%p] variant=%d pos=(%.1f,%.1f) height=%.1f dist=%.1f",
                                     dog,
                                     variant,
                                     worldPosX,
                                     worldPosY,
                                     dogHeight,
                                     dist));
            }
        }
    }

    if (spawnedCount == 0)
    {
        const float fallbackRadius = spawnSettings.maxSpawnDistance * 0.35f;
        const float noise = mMapGenerator.whiteNoise(startX, startY, spawnSettings.salt + 5u);
        const float angle = noise * 2.0f * PI;
        Vector2 fallbackPos = {
            playerPos.x + cosf(angle) * fallbackRadius,
            playerPos.y + sinf(angle) * fallbackRadius
        };

        Dog* dog = new Dog(fallbackPos, 0, DogConstants::DEFAULT_HEIGHT);
        dog->setNavMap(&mNavMap);
        mEnemies.push_back(dog);
        mCollidableEntities.push_back(dog);
        spawnedCount = 1;
        LOG_INFO(TextFormat("Enemy fallback spawn[%p] at (%.1f, %.1f)",
                            dog,
                            fallbackPos.x,
                            fallbackPos.y));
    }

    const double tGenEnd = GetTime();
    LOG_DEBUG(TextFormat("Generated %d enemies in %.2fms",
                         spawnedCount,
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
        generateEnemies();
    }
}

void Level1::ensureTileTexture()
{
    if (mTileTextureReady) return;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlasTexture = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!atlasTexture)
    {
        LOG_ERROR("World atlas texture not loaded; ensureTreeAtlas() succeeded before building tiles.");
        return;
    }

    mTileTexture = atlasTexture;
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
