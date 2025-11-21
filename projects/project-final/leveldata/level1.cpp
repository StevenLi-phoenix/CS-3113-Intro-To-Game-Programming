#include "level1.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include "../lib/ResourceManager.h"

namespace
{
    struct TreeSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
        float minHeightPx = 0.0f;
        float maxHeightPx = 0.0f;
        float baseRootHeight = 0.0f;
        float minRootWidthRatio = 0.0f;
        float maxRootWidthRatio = 0.0f;
    };

    constexpr TreeSpawnSettings TREE_SPAWN_SETTINGS{
        0x3fa8bc91u,
        0.9945f,
        1,
        TreeConstants::MIN_SCALE,
        TreeConstants::MAX_SCALE,
        TreeConstants::ROOT_COLLIDER_HEIGHT,
        TreeConstants::MIN_ROOT_WIDTH_RATIO,
        TreeConstants::MAX_ROOT_WIDTH_RATIO
    };

    struct EnemySpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
        float minHeightPx = 0.0f;
        float maxHeightPx = 0.0f;
        float maxDistanceScale = 1.0f;
    };

    constexpr EnemySpawnSettings ENEMY_SPAWN_SETTINGS{
        0x5f3759d5u,
        0.8f,
        4,
        DogConstants::MIN_HEIGHT,
        DogConstants::MAX_HEIGHT,
        0.85f
    };
}

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    setChunkSize(mChunkSize);
    setChunkLoadRadius(mChunkLoadRadius);
    ensureTreeAtlas();
    ensureTileTexture();
    spawnPlayer();
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

    updateCameraFromPlayer(deltaTime);
    updateGameOverState();
}

void Level1::spawnPlayer()
{
    if (mPlayer)
    {
        mPlayer->setPosition(mPlayerSpawnPosition);
        mPlayer->setMovement({0.0f, 0.0f});
        mPlayer->setVelocity({0.0f, 0.0f});
        mPlayer->setForce({0.0f, 0.0f});
        mPlayer->restoreFullHealth();
        mIsGameOver = false;
        mCamera.target = mPlayer->getPosition();
        return;
    }

    mPlayer = new Player(c::ORIGIN, {54.0f, 75.0f});
    updatePlayerSpawnPoint(mPlayer->getPosition());
    mPlayer->restoreFullHealth();
    mCollidableEntities.push_back(mPlayer);
    mCamera.target = mPlayer->getPosition();
    mIsGameOver = false;
}

void Level1::updateCameraFromPlayer(float deltaTime)
{
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
            if (Dog *dog = dynamic_cast<Dog*>(entity))
            {
                const Vector2 center = dog->getPosition();
                const float dogRadius = std::max(dog->getColliderDimensions().x,
                                                 dog->getColliderDimensions().y) * 0.5f;
                const float playerRadius = (mPlayer)
                                           ? std::max(mPlayer->getColliderDimensions().x,
                                                      mPlayer->getColliderDimensions().y) * 0.5f
                                           : 0.0f;
                const float effectiveRadius = DogConstants::ATTACK_RANGE + dogRadius + playerRadius;
                DrawCircleLinesV(center, effectiveRadius, Fade(RED, 0.7f));
            }
        }
    }
    EndMode2D();

    drawPlayerHUD();
    drawGameOverOverlay();
    DrawFPS(10, 60);
    DrawText("Level 1 - WIP", c::SCREEN_WIDTH / 2 - 100, c::SCREEN_HEIGHT / 2, 24, DARKBLUE);
}

void Level1::shutdown()
{
    LOG_INFO("Level1 shutdown");
    clearMusicNotes();
    clearEnemies();
    clearTrees();
    mCollidableEntities.clear();

    delete mPlayer;
    mPlayer = nullptr;

    delete mMap;
    mMap = nullptr;

    mLevelData.clear();
    mTileTexture = nullptr;
    mTileTextureReady = false;
}

void Level1::clearTrees()
{
    destroyOwnedEntities(mTrees, mCollidableEntities);
}

void Level1::clearEnemies()
{
    for (auto &entry : mChunkEnemies)
    {
        for (Enemy* enemy : entry.second)
        {
            if (!enemy)
            {
                continue;
            }
            removeCollidableEntity(mCollidableEntities, enemy);
            delete enemy;
        }
    }

    mChunkEnemies.clear();
    mEnemies.clear();
}

void Level1::clearMusicNotes()
{
    destroyOwnedEntities(mMusicNotes, mCollidableEntities);
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
        spawnMusicNoteForSlot(mMusicNotes.size());
    }

    refreshMusicNoteSlots();
}

void Level1::spawnMusicNoteForSlot(size_t slotIndex)
{
    if (!mPlayer)
    {
        return;
    }

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
    if (!mPlayer || !mPlayer->getIsActive() || mMusicNotes.empty())
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

void Level1::drawPlayerHUD() const
{
    if (!mPlayer)
    {
        return;
    }

    const float maxHealth = std::max(mPlayer->getMaxHealth(), 0.001f);
    const float healthRatio = std::clamp(mPlayer->getHealth() / maxHealth, 0.0f, 1.0f);
    const Vector2 playerScreenPos = GetWorldToScreen2D(mPlayer->getPosition(), mCamera);

    const Vector2 playerScale = mPlayer->getScale();
    const float barWidth = std::max(playerScale.x * 0.9f, 60.0f);
    const float barHeight = 8.0f;
    const float verticalOffset = -playerScale.y * 0.65f;

    Rectangle barBackground = {
        playerScreenPos.x - barWidth * 0.5f,
        playerScreenPos.y + verticalOffset - barHeight,
        barWidth,
        barHeight
    };

    Rectangle barFill = barBackground;
    barFill.width = barBackground.width * healthRatio;

    DrawRectangleRounded(barBackground, 0.4f, 8, Fade(BLACK, 0.55f));
    DrawRectangleRec(barFill, RED);
    DrawRectangleLinesEx(barBackground, 1.0f, Fade(WHITE, 0.85f));
}

void Level1::updateGameOverState()
{
    if (!mPlayer)
    {
        mIsGameOver = false;
        return;
    }

    mIsGameOver = mPlayer->isDead();
}

void Level1::handleRetryAction()
{
    if (!mIsGameOver)
    {
        return;
    }

    resetPlayerForRetry();
}

void Level1::onRetryBindingChanged(KeyboardKey key)
{
    mRetryBindingKey = key;
}

void Level1::resetPlayerForRetry()
{
    if (!mPlayer)
    {
        return;
    }

    mPlayer->setPosition(mPlayerSpawnPosition);
    mPlayer->setMovement({0.0f, 0.0f});
    mPlayer->setVelocity({0.0f, 0.0f});
    mPlayer->setForce({0.0f, 0.0f});
    mPlayer->restoreFullHealth();
    mAttackTimer = 0.0f;
    mIsGameOver = false;

    updateChunkStream(true);
    mCamera.target = mPlayer->getPosition();
}

void Level1::updatePlayerSpawnPoint(const Vector2 &position)
{
    mPlayerSpawnPosition = position;
}

void Level1::drawGameOverOverlay() const
{
    if (!mIsGameOver || isPaused())
    {
        return;
    }

    DrawRectangle(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, Fade(BLACK, 0.65f));

    const char* title = "You Died";
    const int titleFont = 48;
    const int titleWidth = MeasureText(title, titleFont);
    DrawText(title,
             (c::SCREEN_WIDTH - titleWidth) / 2,
             c::SCREEN_HEIGHT / 2 - 80,
             titleFont,
             RED);

    const std::string retryKeyLabel = KeyToString(mRetryBindingKey);
    const std::string retryText = "Press " + retryKeyLabel + " to Retry";
    const int retryFont = 24;
    const int retryWidth = MeasureText(retryText.c_str(), retryFont);
    DrawText(retryText.c_str(),
             (c::SCREEN_WIDTH - retryWidth) / 2,
             c::SCREEN_HEIGHT / 2,
             retryFont,
             RAYWHITE);

    const char* hint = "Open Settings (F1) to rebind";
    const int hintFont = 20;
    const int hintWidth = MeasureText(hint, hintFont);
    DrawText(hint,
             (c::SCREEN_WIDTH - hintWidth) / 2,
             c::SCREEN_HEIGHT / 2 + 36,
             hintFont,
             LIGHTGRAY);
}

void Level1::buildProceduralMap()
{
    mMapGenerator = MapGenerator(mWorldSeed);

    mMapColumns = getLoadedColumns();
    mMapRows = getLoadedRows();

    MapGenerator::GenerationSettings settings = buildGeneratorSettings();

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

    const Vector2 mapOrigin = computeMapOrigin();
    rebuildMap(mapOrigin);
}

MapGenerator::GenerationSettings Level1::buildGeneratorSettings() const
{
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
    return settings;
}

Vector2 Level1::computeMapOrigin() const
{
    return {
        (static_cast<float>(getChunkStartX()) + static_cast<float>(mMapColumns) / 2.0f) * mTileSize,
        (static_cast<float>(getChunkStartY()) + static_cast<float>(mMapRows) / 2.0f) * mTileSize
    };
}

void Level1::rebuildMap(const Vector2 &origin)
{
    const double tMapStart = GetTime();
    if (mMap)
    {
        mMap->refresh(mLevelData.data(), mMapColumns, mMapRows, origin);
        const double tMapEnd = GetTime();
        LOG_DEBUG(TextFormat("Map refresh (reuse) chunkStart=(%d,%d) took=%.2fms",
                             getChunkStartX(), getChunkStartY(),
                             (tMapEnd - tMapStart) * 1000.0));
        return;
    }

    mMap = new Map(
        mMapColumns,
        mMapRows,
        mLevelData.data(),
        mMapTexturePath,
        mTileSize,
        mTileColumns,
        mTileRows,
        origin,
        mTileAtlasRegion,
        mTileTextureReady ? mTileTexture : nullptr
    );
    const double tMapEnd = GetTime();
    LOG_DEBUG(TextFormat("Map refresh (new map) chunkStart=(%d,%d) took=%.2fms",
                         getChunkStartX(), getChunkStartY(),
                         (tMapEnd - tMapStart) * 1000.0));
}

void Level1::generateTrees()
{
    // Remove old trees from collidable entities
    clearTrees();

    const double tGenStart = GetTime();

    ResourceManager &rm = ResourceManager::instance();
    const std::vector<Rectangle> &treeRects = rm.getSpriteRects(TreeConstants::SPRITE_TAG);
    const int treeVariantCount = std::max(1, static_cast<int>(treeRects.size()));
    if (treeRects.empty())
    {
        LOG_WARNING("Tree sprites missing tag 'TREE' in atlas metadata; using fallback variant.");
    }

    const int startX = getChunkStartX();
    const int startY = getChunkStartY();

    const int spacing = std::max(TREE_SPAWN_SETTINGS.spacing, 1);

    // Generate trees based on world coordinates using deterministic noise
    for (int row = 0; row < mMapRows; row += spacing)
    {
        for (int col = 0; col < mMapColumns; col += spacing)
        {
            const int worldX = startX + col;
            const int worldY = startY + row;

            float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt);

            if (spawnNoise < TREE_SPAWN_SETTINGS.spawnThreshold)
            {
                continue;
            }

            const float variantNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt + 1u);
            const float scaleNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt + 2u);
            const float rootWidthNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt + 3u);
            const float rootHeightNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt + 4u);

            int treeVariant = static_cast<int>(variantNoise * static_cast<float>(treeVariantCount));
            treeVariant = std::clamp(treeVariant, 0, treeVariantCount - 1);

            const float treeHeightPx = TREE_SPAWN_SETTINGS.minHeightPx +
                                       scaleNoise * (TREE_SPAWN_SETTINGS.maxHeightPx - TREE_SPAWN_SETTINGS.minHeightPx);

            const float rootWidthRatio = TREE_SPAWN_SETTINGS.minRootWidthRatio +
                                         rootWidthNoise * (TREE_SPAWN_SETTINGS.maxRootWidthRatio - TREE_SPAWN_SETTINGS.minRootWidthRatio);

            const float rootHeight = TREE_SPAWN_SETTINGS.baseRootHeight *
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
    const double tGenStart = GetTime();
    const int radius = getChunkLoadRadius();
    const int span = getChunkSpan();
    const int minChunkX = mCurrentChunkX - radius;
    const int minChunkY = mCurrentChunkY - radius;

    // Temporarily detach active enemies; keep instances in their chunk buckets.
    for (Enemy* enemy : mEnemies)
    {
        if (!enemy)
        {
            continue;
        }
        enemy->setIsActive(false);
        enemy->setCanCollide(false);
        removeCollidableEntity(mCollidableEntities, enemy);
    }
    mEnemies.clear();

    const int startTileX = getChunkStartX();
    const int startTileY = getChunkStartY();
    Vector2 playerPos = mPlayer ? mPlayer->getPosition()
                                : Vector2{
                                      (static_cast<float>(startTileX) + static_cast<float>(mMapColumns) * 0.5f) * mTileSize,
                                      (static_cast<float>(startTileY) + static_cast<float>(mMapRows) * 0.5f) * mTileSize
                                  };

    auto ensureInCollidables = [this](Enemy* enemy)
    {
        if (!enemy)
        {
            return;
        }
        const bool alreadyPresent = std::find(mCollidableEntities.begin(),
                                              mCollidableEntities.end(),
                                              enemy) != mCollidableEntities.end();
        if (!alreadyPresent)
        {
            mCollidableEntities.push_back(enemy);
        }
    };

    bool hasStoredEnemies = false;

    for (int dy = 0; dy < span; ++dy)
    {
        for (int dx = 0; dx < span; ++dx)
        {
            const std::pair<int, int> chunkKey{
                minChunkX + dx,
                minChunkY + dy
            };

            std::vector<Enemy*> &bucket = mChunkEnemies[chunkKey];
            if (bucket.empty())
            {
                spawnEnemiesForChunk(chunkKey, bucket, playerPos);
            }
            if (!bucket.empty())
            {
                hasStoredEnemies = true;
            }

            for (Enemy* enemy : bucket)
            {
                if (!enemy)
                {
                    continue;
                }

                if (!enemy->isDead())
                {
                    enemy->setIsActive(true);
                    enemy->setCanCollide(true);
                }

                ensureInCollidables(enemy);
                mEnemies.push_back(enemy);
            }
        }
    }

    if (mEnemies.empty() && !hasStoredEnemies)
    {
        const float chunkWorldSize = static_cast<float>(mChunkSize) * mTileSize;
        const float fallbackRadius = chunkWorldSize * 0.35f;
        const int chunkBaseX = mCurrentChunkX * mChunkSize;
        const int chunkBaseY = mCurrentChunkY * mChunkSize;
        const float noise = mMapGenerator.whiteNoise(chunkBaseX, chunkBaseY, ENEMY_SPAWN_SETTINGS.salt + 5u);
        const float angle = noise * 2.0f * PI;
        Vector2 fallbackPos = {
            playerPos.x + cosf(angle) * fallbackRadius,
            playerPos.y + sinf(angle) * fallbackRadius
        };

        const std::pair<int, int> currentChunk{mCurrentChunkX, mCurrentChunkY};
        std::vector<Enemy*> &bucket = mChunkEnemies[currentChunk];
        Dog* dog = new Dog(fallbackPos, 0, DogConstants::DEFAULT_HEIGHT);
        dog->setNavMap(&mNavMap);
        bucket.push_back(dog);
        mEnemies.push_back(dog);
        mCollidableEntities.push_back(dog);
        LOG_INFO(TextFormat("Enemy fallback spawn[%p] at (%.1f, %.1f)",
                            dog,
                            fallbackPos.x,
                            fallbackPos.y));
    }

    const double tGenEnd = GetTime();
    LOG_DEBUG(TextFormat("Activated %d enemies across %d chunks in %.2fms",
                         static_cast<int>(mEnemies.size()),
                         span * span,
                         (tGenEnd - tGenStart) * 1000.0));
}

void Level1::spawnEnemiesForChunk(const std::pair<int, int> &chunk,
                                  std::vector<Enemy*> &bucket,
                                  const Vector2 &playerPos)
{
    const int spacing = std::max(ENEMY_SPAWN_SETTINGS.spacing, 1);
    const int chunkTileStartX = chunk.first * mChunkSize;
    const int chunkTileStartY = chunk.second * mChunkSize;
    const float chunkWorldSize = static_cast<float>(mChunkSize) * mTileSize;
    const float maxSpawnDistance = chunkWorldSize * ENEMY_SPAWN_SETTINGS.maxDistanceScale;
    const float maxSpawnDistanceSq = maxSpawnDistance * maxSpawnDistance;

    int spawnedCount = 0;
    for (int row = 0; row < mChunkSize; row += spacing)
    {
        for (int col = 0; col < mChunkSize; col += spacing)
        {
            const int worldX = chunkTileStartX + col;
            const int worldY = chunkTileStartY + row;

            float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, ENEMY_SPAWN_SETTINGS.salt);
            if (spawnNoise < ENEMY_SPAWN_SETTINGS.spawnThreshold)
            {
                continue;
            }

            const float variantNoise = mMapGenerator.whiteNoise(worldX, worldY, ENEMY_SPAWN_SETTINGS.salt + 1u);
            const float heightNoise = mMapGenerator.whiteNoise(worldX, worldY, ENEMY_SPAWN_SETTINGS.salt + 2u);

            int variant = static_cast<int>(variantNoise * static_cast<float>(DogConstants::VARIANT_COUNT));
            variant = std::clamp(variant, 0, DogConstants::VARIANT_COUNT - 1);

            const float dogHeight = ENEMY_SPAWN_SETTINGS.minHeightPx +
                                    heightNoise * (ENEMY_SPAWN_SETTINGS.maxHeightPx - ENEMY_SPAWN_SETTINGS.minHeightPx);

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
            bucket.push_back(dog);
            spawnedCount++;

            if (isDebugMode())
            {
                const float dist = sqrtf((dx * dx) + (dy * dy));
                LOG_DEBUG(TextFormat("Enemy spawn[%p] chunk=(%d,%d) variant=%d pos=(%.1f,%.1f) height=%.1f dist=%.1f",
                                     dog,
                                     chunk.first,
                                     chunk.second,
                                     variant,
                                     worldPosX,
                                     worldPosY,
                                     dogHeight,
                                     dist));
            }
        }
    }

    if (isDebugMode())
    {
        LOG_DEBUG(TextFormat("Chunk (%d,%d) spawned %d enemies",
                             chunk.first,
                             chunk.second,
                             spawnedCount));
    }
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
