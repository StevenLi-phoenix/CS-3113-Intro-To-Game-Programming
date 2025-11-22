#include "level1.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include "../lib/ResourceManager.h"
#include "../lib/Inventory.h"
#include "../lib/ui/InventoryBar.h"
#include "branch.h"
#include "box.h"
#include "compass.h"
#include "goldcoin.h"
#include "rock.h"
#include "table_with_map.h"
#include "LevelSelectScene.h"
#include "../lib/SceneController.h"

extern SceneController* gSceneController;

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

    struct BoxSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
    };

    constexpr BoxSpawnSettings BOX_SPAWN_SETTINGS{
        0x91b58badu,
        0.985f,
        8
    };

    struct RockSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
        float minHeightPx = 0.0f;
        float maxHeightPx = 0.0f;
        float minColliderRatio = 0.0f;
        float maxColliderRatio = 0.0f;
        float safeRadiusTiles = 0.0f;
    };

    constexpr RockSpawnSettings ROCK_SPAWN_SETTINGS{
        0xd137f0a5u,
        0.95f,
        4,
        32.0f,
        72.0f,
        0.5f,
        0.85f,
        2.5f
    };

    constexpr const char *ROCK_SPRITE_TAGS[] = {
        "SMALLROCK",
        "SMALLROCK2",
        "SMALLROCK3",
        "LARGEROCK"
    };

    constexpr const char *BRANCH_SLOT_ICON_TAG = tags::BRANCH;
    constexpr const char *GOLD_SLOT_ICON_TAG = tags::GOLDCOIN;
    constexpr float GOLD_PICKUP_RADIUS = 42.0f;
    constexpr float GOLD_DROP_CHANCE = 0.5f;
    constexpr float GOLD_DROP_OFFSET_MIN = 6.0f;
    constexpr float GOLD_DROP_OFFSET_MAX = 14.0f;
    constexpr float TABLE_DISTANCE_MIN_TILES = 64.0f;
    constexpr float TABLE_DISTANCE_MAX_TILES = 256.0f;

    namespace tutorial
    {
        constexpr float AUTO_HIDE_SECONDS = 10.0f;
        constexpr float FADE_SECONDS = 1.0f;
        constexpr int MAX_GAMEPADS = 4;
        constexpr const char *TITLE = "Getting Started";
        constexpr const char *LINES[] = {
            "WASD or Arrow Keys to move your character",
            "Left click anywhere to toss a branch at that spot",
            "Press Z to auto-throw at the nearest enemy, X for melee",
            "Press F1 for settings, rebinding, and tips",
            "Seek the table with a map to reach the next level"
        };
        constexpr size_t LINE_COUNT = sizeof(LINES) / sizeof(LINES[0]);
    }
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
    applyDifficulty(mDifficulty);
    resetBranchInventory();
    initialiseInventoryUI();
    spawnQuestTarget();
    if (!mCompassUI)
    {
        mCompassUI = std::make_unique<Compass>();
    }
    mCompassUI->setTarget(mTable.get());
    mCompassUI->setPlayer(mPlayer);
    mCompassUI->setCamera(&mCamera);
    mCompassUI->setDistanceDisplay(mTileSize,
                                   TABLE_DISTANCE_MIN_TILES,
                                   TABLE_DISTANCE_MAX_TILES);
    mSkipPlayerChunkForNextEnemySpawn = true;
    updateChunkStream(true);
    // Lighting shader temporarily disabled; keep call commented for future restoration.
    // initialiseLightingShader();

    mTutorialOverlayVisible = true;
    mTutorialOverlayDismissed = false;
    mTutorialOverlayDisplayTimer = 0.0f;
    mTutorialOverlayFadeTimer = 0.0f;
}

void Level1::update(float deltaTime)
{
    advanceDayNightCycle(deltaTime);
    updateChunkStream();
    ensureMusicNotes();
    updateMusicNoteBehaviour();
    handleMouseBranchInput();

    for (Entity* entity : mCollidableEntities)
    {
        entity->update(deltaTime, mPlayer, mMap, mCollidableEntities);
    }

    resolveBranchImpacts();
    cleanupBranches();
    cleanupInactiveTrees();
    updateBoxRewards();
    updateGoldCoins();
    updatePlayerAttack(deltaTime);
    updateMeleeTimer(deltaTime);

    updateCameraFromPlayer(deltaTime);
    updateGameOverState();
    updateInventoryUI(deltaTime);
    updateTutorialOverlay(deltaTime);
    if (mCompassUI)
    {
        mCompassUI->update(deltaTime, mPlayer, mMap, mCollidableEntities);
    }
    updateQuestState();
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
    resetBranchInventory();
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
    // Shader temporarily disabled; keep block for future re-enable.
    // ShaderProgram* lightingShader = getLightingShader();
    // Vector2 screenResolution = { static_cast<float>(c::SCREEN_WIDTH), static_cast<float>(c::SCREEN_HEIGHT) };
    // if (lightingShader)
    // {
    //     const float nightFactor = getNightFactor();
    //     const float ambient = getAmbientLight();
    //     const float shadowStrength = lighting::SHADOW_BASE_STRENGTH * getShadowFactor();
    //     const float vignetteRadius = lighting::VIGNETTE_DAY_RADIUS +
    //                                  (lighting::VIGNETTE_NIGHT_RADIUS - lighting::VIGNETTE_DAY_RADIUS) * nightFactor;
    //     Vector2 focusScreen = { screenResolution.x * 0.5f, screenResolution.y * 0.5f };
    //     if (mPlayer)
    //     {
    //         focusScreen = GetWorldToScreen2D(mPlayer->getPosition(), mCamera);
    //     }
    //     Vector2 focusNormalized = {
    //         std::clamp(focusScreen.x / screenResolution.x, 0.0f, 1.0f),
    //         std::clamp(focusScreen.y / screenResolution.y, 0.0f, 1.0f)
    //     };
    //     focusNormalized.y = 1.0f - focusNormalized.y;

    //     Vector2 cursorScreen = GetMousePosition();
    //     Vector2 cursorNormalized = {
    //         std::clamp(cursorScreen.x / screenResolution.x, 0.0f, 1.0f),
    //         std::clamp(cursorScreen.y / screenResolution.y, 0.0f, 1.0f)
    //     };
    //     cursorNormalized.y = 1.0f - cursorNormalized.y;

    //     lightingShader->setFloat("u_timeOfDay", getTimeOfDay());
    //     lightingShader->setFloat("u_lightIntensity", ambient);
    //     lightingShader->setFloat("u_shadowStrength", shadowStrength);
    //     lightingShader->setFloat("u_vignetteRadius", vignetteRadius);
    //     lightingShader->setFloat("u_vignetteSoftness", lighting::VIGNETTE_SOFTNESS);
    //     lightingShader->setFloat("u_vignetteIntensity", nightFactor);
    //     lightingShader->setVector2("u_resolution", screenResolution);
    //     lightingShader->setVector2("u_focus", focusNormalized);
    //     lightingShader->setVector2("u_cursor", cursorNormalized);
    //     lightingShader->setFloat("u_cursorRadius", lighting::CURSOR_LIGHT_RADIUS);
    //     lightingShader->setFloat("u_cursorIntensity", lighting::CURSOR_LIGHT_INTENSITY);

    //     lightingShader->begin();
    // }

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
        mNavMap.clearDebugPaths();
        for (Enemy* enemy : mEnemies)
        {
            if (!enemy || !enemy->hasActivePath())
            {
                continue;
            }
            std::vector<Vector2> path = enemy->activePathPoints();
            if (path.size() >= 2)
            {
                mNavMap.addDebugPath(path, BLUE);
            }
        }
        mNavMap.debugRender();
    }
    EndMode2D();

    // if (lightingShader)
    // {
    //     lightingShader->end();
    // }

    drawPlayerHUD();
    drawInventoryOverlay();
    drawCompassIndicator();
    drawQuestLog();
    drawTutorialOverlay();
    drawGameOverOverlay();
    DrawFPS(0, 60);
}

void Level1::shutdown()
{
    LOG_INFO("Level1 shutdown");
    mInventoryBar.reset();
    mInventory.reset();
    mCompassUI.reset();
    mTable.reset();
    clearMusicNotes();
    clearEnemies();
    clearBranches();
    clearGoldCoins();
    clearBoxes();
    clearTrees();
    clearRocks();
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
    for (Tree* tree : mActiveTrees)
    {
        if (!tree)
        {
            continue;
        }
        removeCollidableEntity(mCollidableEntities, tree);
    }
    mActiveTrees.clear();

    for (auto &entry : mChunkTrees)
    {
        for (Tree* tree : entry.second)
        {
            if (!tree)
            {
                continue;
            }
            removeCollidableEntity(mCollidableEntities, tree);
            delete tree;
        }
        entry.second.clear();
    }
    mChunkTrees.clear();
}

void Level1::clearRocks()
{
    destroyOwnedEntities(mRocks, mCollidableEntities);
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

void Level1::cleanupInactiveTrees()
{
    auto it = mActiveTrees.begin();
    while (it != mActiveTrees.end())
    {
        Tree* tree = *it;
        if (!tree || tree->isDead())
        {
            if (tree)
            {
                tree->setIsActive(false);
                tree->setCanCollide(false);
                removeCollidableEntity(mCollidableEntities, tree);
            }
            it = mActiveTrees.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::clearBranches()
{
    destroyOwnedEntities(mBranches, mCollidableEntities);
}

void Level1::clearGoldCoins(bool resetCount)
{
    destroyOwnedEntities(mGoldCoins, mCollidableEntities);
    mGoldCoins.clear();
    if (resetCount)
    {
        mGoldCount = 0;
        syncGoldSlot();
    }
}

void Level1::clearBoxes()
{
    for (auto &entry : mChunkBoxes)
    {
        for (Box* box : entry.second)
        {
            if (!box)
            {
                continue;
            }
            removeCollidableEntity(mCollidableEntities, box);
            delete box;
        }
    }
    mChunkBoxes.clear();
    mBoxes.clear();
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
    if (defeated)
    {
        handleEnemyDefeated(target);
        if (isDebugMode())
        {
            LOG_DEBUG(TextFormat("Music attack defeated enemy[%p] damage=%d", target, damage));
        }
    }

    note->launchAttack(target->getPosition());
    advanceNoteVariant();
    mAttackTimer = std::max(mAttackIntervalSeconds, 0.01f);
}

void Level1::handleMouseBranchInput()
{
    if (!mPlayer || mIsGameOver)
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!isBranchSelected() || isCompassSelected())
        {
            return;
        }
        Vector2 cursor = GetMousePosition();
        Vector2 worldTarget = GetScreenToWorld2D(cursor, mCamera);
        tryThrowBranchAt(worldTarget);
    }
}

bool Level1::tryThrowBranchAt(const Vector2 &worldTarget)
{
    if (!mPlayer || mBranchInventory <= 0)
    {
        return false;
    }

    Vector2 start = mPlayer->getPosition();
    Vector2 toTarget = {
        worldTarget.x - start.x,
        worldTarget.y - start.y
    };
    const float distance = Vector2Length(toTarget);
    if (distance < branch::MIN_THROW_DISTANCE)
    {
        return false;
    }
    Vector2 direction = Vector2Normalize(toTarget);
    const float travelDistance = std::min(distance, branch::THROW_RANGE);
    if (!consumeBranch())
    {
        return false;
    }

    Branch *projectile = new Branch(start, direction, travelDistance);
    mBranches.push_back(projectile);
    mCollidableEntities.push_back(projectile);
    return true;
}

bool Level1::tryThrowBranchAtEnemy()
{
    Enemy* target = findNearestEnemy(branch::THROW_RANGE);
    if (!target)
    {
        return false;
    }
    return tryThrowBranchAt(target->getPosition());
}

void Level1::resolveBranchImpacts()
{
    if (mBranches.empty())
    {
        return;
    }

    for (Branch* projectile : mBranches)
    {
        if (!projectile || projectile->isSpent() || !projectile->getIsActive())
        {
            continue;
        }

        for (Enemy* enemy : mEnemies)
        {
            if (!enemy || !enemy->getIsActive())
            {
                continue;
            }

            if (projectile->intersects(*enemy))
            {
                const bool defeated = enemy->applyDamage(projectile->getDamage());
                if (defeated)
                {
                    handleEnemyDefeated(enemy);
                }
                projectile->markSpent();
                break;
            }
        }
    }
}

void Level1::cleanupBranches()
{
    auto it = mBranches.begin();
    while (it != mBranches.end())
    {
        Branch* projectile = *it;
        if (!projectile || projectile->isSpent() || !projectile->getIsActive())
        {
            removeCollidableEntity(mCollidableEntities, projectile);
            delete projectile;
            it = mBranches.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::resetBranchInventory()
{
    mBranchCapacity = std::max(branch::MAX_HELD, mBranchCapacity);
    mBranchInventory = std::clamp(mInitialBranchCount, 0, mBranchCapacity);
    syncBranchSlot();
}

bool Level1::consumeBranch()
{
    if (mBranchInventory <= 0)
    {
        return false;
    }
    --mBranchInventory;
    syncBranchSlot();
    return true;
}

void Level1::addBranches(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    mBranchInventory = std::clamp(mBranchInventory + amount, 0, mBranchCapacity);
    syncBranchSlot();
}

void Level1::applyDifficulty(const DifficultyState &state)
{
    mDifficulty = state;
    mInitialBranchCount = std::clamp(mDifficulty.initialBranches(), 0, branch::MAX_HELD);
    mBoxBranchReward = std::max(1, mDifficulty.boxReward());
}

void Level1::spawnGoldCoin(const Vector2 &position)
{
    GoldCoin *coin = new GoldCoin(position);
    mGoldCoins.push_back(coin);
    mCollidableEntities.push_back(coin);
}

void Level1::updateGoldCoins()
{
    if (!mPlayer)
    {
        return;
    }

    const float radiusSq = GOLD_PICKUP_RADIUS * GOLD_PICKUP_RADIUS;
    auto it = mGoldCoins.begin();
    while (it != mGoldCoins.end())
    {
        GoldCoin *coin = *it;
        if (!coin || !coin->getIsActive())
        {
            if (coin)
            {
                removeCollidableEntity(mCollidableEntities, coin);
                delete coin;
            }
            it = mGoldCoins.erase(it);
            continue;
        }

        Vector2 diff = {
            coin->getPosition().x - mPlayer->getPosition().x,
            coin->getPosition().y - mPlayer->getPosition().y
        };
        const float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq <= radiusSq)
        {
            collectGoldCoin(coin);
            it = mGoldCoins.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::collectGoldCoin(GoldCoin *coin)
{
    if (!coin)
    {
        return;
    }
    coin->setIsActive(false);
    removeCollidableEntity(mCollidableEntities, coin);
    delete coin;
    ++mGoldCount;
    syncGoldSlot();
}

void Level1::handleEnemyDefeated(Enemy *enemy)
{
    if (!enemy)
    {
        return;
    }

    const float dropRoll = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f;
    if (dropRoll > GOLD_DROP_CHANCE)
    {
        return;
    }

    const float angle = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f * 2.0f * PI;
    const float radius = GOLD_DROP_OFFSET_MIN +
                         (static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f) *
                         (GOLD_DROP_OFFSET_MAX - GOLD_DROP_OFFSET_MIN);
    Vector2 dropPos = {
        enemy->getPosition().x + cosf(angle) * radius,
        enemy->getPosition().y + sinf(angle) * radius
    };
    spawnGoldCoin(dropPos);

    if (isDebugMode())
    {
        LOG_DEBUG(TextFormat("Enemy[%p] dropped gold at (%.1f, %.1f)",
                             enemy,
                             dropPos.x,
                             dropPos.y));
    }
}

void Level1::updateMeleeTimer(float deltaTime)
{
    if (mMeleeTimer > 0.0f)
    {
        mMeleeTimer = std::max(0.0f, mMeleeTimer - deltaTime);
    }
}

void Level1::tryMeleeAttack()
{
    if (!mPlayer || mMeleeTimer > 0.0f)
    {
        return;
    }

    Enemy* target = findNearestMeleeTarget(mMeleeRange);
    if (!target)
    {
        return;
    }

    applyMeleeDamage(target);
    mMeleeTimer = std::max(mMeleeCooldown, 0.05f);
}

Enemy* Level1::findNearestMeleeTarget(float range) const
{
    if (!mPlayer)
    {
        return nullptr;
    }

    const float limitSq = range * range;
    const Vector2 playerPos = mPlayer->getPosition();
    Enemy* closest = nullptr;
    float bestDistanceSq = limitSq;

    auto consider = [&](Enemy* candidate)
    {
        if (!candidate || !candidate->getIsActive())
        {
            return;
        }
        Vector2 diff = {
            candidate->getPosition().x - playerPos.x,
            candidate->getPosition().y - playerPos.y
        };
        const float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq <= bestDistanceSq)
        {
            bestDistanceSq = distSq;
            closest = candidate;
        }
    };

    for (Enemy* enemy : mEnemies)
    {
        consider(enemy);
    }

    for (Tree* tree : mActiveTrees)
    {
        consider(tree);
    }

    return closest;
}

void Level1::applyMeleeDamage(Enemy *target)
{
    if (!target)
    {
        return;
    }

    if (Tree* tree = dynamic_cast<Tree*>(target))
    {
        tree->applyDamage(1.0f);
        addBranches(1);
    }
    else
    {
        const bool defeated = target->applyDamage(mMeleeDamage);
        if (defeated)
        {
            handleEnemyDefeated(target);
        }
    }
}

void Level1::updateBoxRewards()
{
    if (!mPlayer)
    {
        return;
    }

    auto it = mBoxes.begin();
    while (it != mBoxes.end())
    {
        Box* box = *it;
        if (!box)
        {
            it = mBoxes.erase(it);
            continue;
        }

        if (box->isCollected())
        {
            it = mBoxes.erase(it);
            continue;
        }

        if (mPlayer->intersects(*box))
        {
            collectBox(box);
            it = mBoxes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::collectBox(Box *box)
{
    if (!box)
    {
        return;
    }

    box->markCollected();
    removeCollidableEntity(mCollidableEntities, box);
    addBranches(mBoxBranchReward);

    if (isDebugMode())
    {
        LOG_INFO(TextFormat("Box[%p] collected. +%d branches (total=%d)",
                            box,
                            mBoxBranchReward,
                            mBranchInventory));
    }
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

void Level1::drawInventoryOverlay()
{
    if (mInventoryBar)
    {
        mInventoryBar->render();
    }
}

void Level1::updateInventoryUI(float deltaTime)
{
    if (mInventoryBar)
    {
        mInventoryBar->update(deltaTime);
    }
}

void Level1::updateTutorialOverlay(float deltaTime)
{
    if (!mTutorialOverlayVisible)
    {
        return;
    }

    mTutorialOverlayDisplayTimer += deltaTime;

    if (!mTutorialOverlayDismissed)
    {
        const bool interacted = tutorialInputDetected();
        if (interacted || mTutorialOverlayDisplayTimer >= tutorial::AUTO_HIDE_SECONDS)
        {
            mTutorialOverlayDismissed = true;
            mTutorialOverlayFadeTimer = 0.0f;
        }
        return;
    }

    mTutorialOverlayFadeTimer += deltaTime;
    if (mTutorialOverlayFadeTimer >= tutorial::FADE_SECONDS)
    {
        mTutorialOverlayVisible = false;
    }
}

void Level1::drawTutorialOverlay() const
{
    if (!mTutorialOverlayVisible || isPaused())
    {
        return;
    }

    const float alpha = tutorialOverlayAlpha();
    if (alpha <= 0.0f)
    {
        return;
    }

    const float headerHeight = 70.0f;
    const float lineSpacing = 28.0f;
    const float footerHeight = 40.0f;
    const float panelHeight = headerHeight +
                              static_cast<float>(tutorial::LINE_COUNT) * lineSpacing +
                              footerHeight;

    DrawRectangle(0,
                  0,
                  c::SCREEN_WIDTH,
                  static_cast<int>(panelHeight),
                  Fade(BLACK, 0.55f * alpha));

    const int titleFontSize = 32;
    const int titleWidth = MeasureText(tutorial::TITLE, titleFontSize);
    DrawText(tutorial::TITLE,
             (c::SCREEN_WIDTH - titleWidth) / 2,
             18,
             titleFontSize,
             Fade(RAYWHITE, alpha));

    int lineY = 70;
    const int lineFontSize = 22;
    for (size_t i = 0; i < tutorial::LINE_COUNT; ++i)
    {
        const char *line = tutorial::LINES[i];
        const int lineWidth = MeasureText(line, lineFontSize);
        DrawText(line,
                 (c::SCREEN_WIDTH - lineWidth) / 2,
                 lineY,
                 lineFontSize,
                 Fade(LIGHTGRAY, alpha));
        lineY += static_cast<int>(lineSpacing);
    }

    const char *dismissHint = "Move, click, or wait a moment to hide this hint";
    const int hintFontSize = 18;
    const int hintWidth = MeasureText(dismissHint, hintFontSize);
    DrawText(dismissHint,
             (c::SCREEN_WIDTH - hintWidth) / 2,
             lineY,
             hintFontSize,
             Fade(GRAY, alpha));
}

bool Level1::tutorialInputDetected() const
{
    for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; ++key)
    {
        if (IsKeyPressed(static_cast<KeyboardKey>(key)))
        {
            return true;
        }
    }

    constexpr MouseButton mouseButtons[] = {
        MOUSE_BUTTON_LEFT,
        MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_MIDDLE
    };
    for (MouseButton button : mouseButtons)
    {
        if (IsMouseButtonPressed(button))
        {
            return true;
        }
    }

    for (int pad = 0; pad < tutorial::MAX_GAMEPADS; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }

        for (int button = GAMEPAD_BUTTON_UNKNOWN + 1;
             button <= GAMEPAD_BUTTON_RIGHT_THUMB;
             ++button)
        {
            if (IsGamepadButtonPressed(pad, static_cast<GamepadButton>(button)))
            {
                return true;
            }
        }

        const int axisCount = GetGamepadAxisCount(pad);
        for (int axis = 0; axis < axisCount; ++axis)
        {
            if (std::fabs(GetGamepadAxisMovement(pad, axis)) > 0.35f)
            {
                return true;
            }
        }
    }

    return false;
}

float Level1::tutorialOverlayAlpha() const
{
    if (!mTutorialOverlayVisible)
    {
        return 0.0f;
    }
    if (!mTutorialOverlayDismissed)
    {
        return 1.0f;
    }
    const float remaining = 1.0f - (mTutorialOverlayFadeTimer / tutorial::FADE_SECONDS);
    return std::clamp(remaining, 0.0f, 1.0f);
}

void Level1::initialiseInventoryUI()
{
    const size_t slotCount = 4;
    mInventory = std::make_unique<Inventory>(slotCount);
    mAxeSlotIndex = 0;
    mCompassSlotIndex = 1;
    mBranchSlotIndex = 2;
    mGoldSlotIndex = 3;
    mGoldCount = 0;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);

    auto applyIcon = [&](InventorySlot &slot, const char *tag)
    {
        if (!atlas)
        {
            return;
        }
        Rectangle rect = rm.getSpriteRect(tag);
        if (rect.width > 0.0f && rect.height > 0.0f)
        {
            slot.icon = atlas;
            slot.iconSource = rect;
        }
    };

    InventorySlot axeSlot;
    axeSlot.id = "axe";
    axeSlot.label = "Axe";
    axeSlot.iconTint = Fade(WHITE, 0.9f);
    axeSlot.quantity = 1;
    applyIcon(axeSlot, tags::AXE);
    mInventory->setSlot(mAxeSlotIndex, axeSlot);

    InventorySlot compassSlot;
    compassSlot.id = "compass";
    compassSlot.label = "Compass";
    compassSlot.iconTint = Fade(WHITE, 0.95f);
    compassSlot.quantity = 1;
    applyIcon(compassSlot, tags::COMPASS);
    mInventory->setSlot(mCompassSlotIndex, compassSlot);

    InventorySlot branchSlot;
    branchSlot.id = "branches";
    branchSlot.label = "Branches";
    branchSlot.iconTint = DARKGREEN;
    branchSlot.quantity = mBranchInventory;
    applyIcon(branchSlot, BRANCH_SLOT_ICON_TAG);
    mInventory->setSlot(mBranchSlotIndex, branchSlot);

    InventorySlot goldSlot;
    goldSlot.id = "goldcoin";
    goldSlot.label = "Gold";
    goldSlot.iconTint = GOLD;
    goldSlot.quantity = mGoldCount;
    applyIcon(goldSlot, GOLD_SLOT_ICON_TAG);
    mInventory->setSlot(mGoldSlotIndex, goldSlot);

    mInventory->setSelectedIndex(mBranchSlotIndex);

    if (!mInventoryBar)
    {
        mInventoryBar = std::make_unique<InventoryBar>(mInventory.get());
    }
    else
    {
        mInventoryBar->setInventory(mInventory.get());
    }

    syncBranchSlot();
    syncGoldSlot();
}

void Level1::syncBranchSlot()
{
    if (!mInventory)
    {
        return;
    }

    if (mInventory->getSlotCount() == 0)
    {
        return;
    }

    if (mBranchSlotIndex >= mInventory->getSlotCount())
    {
        mBranchSlotIndex = 0;
    }

    InventorySlot slot = mInventory->getSlot(mBranchSlotIndex);
    slot.id = "branches";
    if (slot.label.empty())
    {
        slot.label = "Branches";
    }
    slot.iconTint = DARKGREEN;
    slot.quantity = mBranchInventory;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (atlas)
    {
        Rectangle iconRect = rm.getSpriteRect(BRANCH_SLOT_ICON_TAG);
        if (iconRect.width > 0.0f && iconRect.height > 0.0f)
        {
            slot.icon = atlas;
            slot.iconSource = iconRect;
        }
        else
        {
            slot.icon = nullptr;
            slot.iconSource = {0.0f, 0.0f, 0.0f, 0.0f};
        }
    }
    else
    {
        slot.icon = nullptr;
        slot.iconSource = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    mInventory->setSlot(mBranchSlotIndex, slot);
}

void Level1::syncGoldSlot()
{
    if (!mInventory)
    {
        return;
    }

    if (mInventory->getSlotCount() == 0)
    {
        return;
    }

    if (mGoldSlotIndex >= mInventory->getSlotCount())
    {
        mGoldSlotIndex = mInventory->getSlotCount() - 1;
    }

    InventorySlot slot = mInventory->getSlot(mGoldSlotIndex);
    slot.id = "goldcoin";
    if (slot.label.empty())
    {
        slot.label = "Gold";
    }
    slot.iconTint = GOLD;
    slot.quantity = mGoldCount;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (atlas)
    {
        Rectangle iconRect = rm.getSpriteRect(GOLD_SLOT_ICON_TAG);
        if (iconRect.width > 0.0f && iconRect.height > 0.0f)
        {
            slot.icon = atlas;
            slot.iconSource = iconRect;
        }
        else
        {
            slot.icon = nullptr;
            slot.iconSource = {0.0f, 0.0f, 0.0f, 0.0f};
        }
    }
    else
    {
        slot.icon = nullptr;
        slot.iconSource = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    mInventory->setSlot(mGoldSlotIndex, slot);
}

bool Level1::isBranchSelected() const
{
    return mInventory && mInventory->getSelectedIndex() == mBranchSlotIndex;
}

bool Level1::isCompassSelected() const
{
    return mInventory && mInventory->getSelectedIndex() == mCompassSlotIndex;
}

bool Level1::isAxeSelected() const
{
    return mInventory && mInventory->getSelectedIndex() == mAxeSlotIndex;
}

void Level1::spawnQuestTarget()
{
    mQuestComplete = false;
    clearGoldCoins(false);

    if (!mTable)
    {
        mTable = std::make_unique<TableWithMap>();
    }

    const Vector2 basePos = mPlayer ? mPlayer->getPosition() : mPlayerSpawnPosition;
    const float chunkWorld = static_cast<float>(mChunkSize) * mTileSize;
    const int minChunks = std::max(1, 1 + mDifficulty.index); // at least 1 chunk (64 tiles) away, slightly further on harder presets
    const float minDistance = chunkWorld * static_cast<float>(minChunks);
    const float noise = mMapGenerator.whiteNoise(static_cast<int>(basePos.x), static_cast<int>(basePos.y), mWorldSeed);
    const float angle = noise * 2.0f * PI;
    Vector2 dir = { cosf(angle), sinf(angle) };
    if (Vector2LengthSqr(dir) < 0.0001f)
    {
        dir = { 1.0f, 0.0f };
    }
    dir = Vector2Normalize(dir);
    Vector2 targetPos = {
        basePos.x + dir.x * minDistance,
        basePos.y + dir.y * minDistance
    };
    mTable->setPosition(targetPos);
    mTable->setIsActive(true);
    mTable->setCanCollide(true);

    const bool alreadyPresent = std::find(mCollidableEntities.begin(),
                                          mCollidableEntities.end(),
                                          mTable.get()) != mCollidableEntities.end();
    if (!alreadyPresent)
    {
        mCollidableEntities.push_back(mTable.get());
    }
}

int Level1::requiredGold() const
{
    return mDifficulty.goldRequirement();
}

void Level1::updateQuestState()
{
    if (mQuestComplete || !mPlayer || !mTable)
    {
        return;
    }

    const int goldNeeded = requiredGold();
    if (goldNeeded > 0 && mGoldCount < goldNeeded)
    {
        return;
    }

    if (mPlayer->intersects(*mTable))
    {
        mQuestComplete = true;
        if (gSceneController)
        {
            gSceneController->requestSceneChange(std::make_unique<LevelSelectScene>());
        }
    }
}

void Level1::drawQuestLog() const
{
    const float panelWidth = 340.0f;
    const float panelHeight = 150.0f;
    Rectangle panel = {
        16.0f,
        16.0f,
        panelWidth,
        panelHeight
    };
    DrawRectangleRounded(panel, 0.2f, 6, Fade(BLACK, 0.55f));
    DrawRectangleLinesEx(panel, 2.0f, Fade(WHITE, 0.4f));

    const char *title = "Quest Log";
    const int titleSize = 20;
    const int titleWidth = MeasureText(title, titleSize);
    DrawText(title,
             static_cast<int>(panel.x + 12.0f),
             static_cast<int>(panel.y + 10.0f),
             titleSize,
             RAYWHITE);

    const int goldNeeded = requiredGold();
    const bool hasEnoughGold = goldNeeded <= 0 || mGoldCount >= goldNeeded;
    const std::string status = mQuestComplete ? "Complete" : (hasEnoughGold ? "Ready" : "Collect");
    const Color statusColor = mQuestComplete ? LIME : (hasEnoughGold ? SKYBLUE : YELLOW);
    const int statusSize = 18;
    DrawText(status.c_str(),
             static_cast<int>(panel.x + panel.width - 84.0f),
             static_cast<int>(panel.y + 12.0f),
             statusSize,
             statusColor);

    const int descSize = 18;
    const std::string line = mQuestDescription + (mQuestComplete ? " - Done" : "");
    DrawText(line.c_str(),
             static_cast<int>(panel.x + 12.0f),
             static_cast<int>(panel.y + 40.0f),
             descSize,
             Fade(LIGHTGRAY, 0.95f));

    int textY = static_cast<int>(panel.y + 64.0f);
    if (mPlayer && mTable)
    {
        const float worldDistance = Vector2Distance(mPlayer->getPosition(), mTable->getPosition());
        const float tileDistance = worldDistance / mTileSize;
        const float clampedTiles = std::clamp(tileDistance,
                                              TABLE_DISTANCE_MIN_TILES,
                                              TABLE_DISTANCE_MAX_TILES);
        const int displayDistance = static_cast<int>(std::round(clampedTiles));
        const bool cappedHigh = tileDistance > TABLE_DISTANCE_MAX_TILES + 0.01f;
        const bool cappedLow = tileDistance < TABLE_DISTANCE_MIN_TILES - 0.01f;
        const char *distanceSuffix = cappedHigh ? "+" : (cappedLow ? "-" : "");
        const char *distanceLine = TextFormat("Map table distance: %d tiles%s",
                                              displayDistance,
                                              distanceSuffix);
        DrawText(distanceLine,
                 static_cast<int>(panel.x + 12.0f),
                 textY,
                 16,
                 Fade(SKYBLUE, 0.95f));
        textY += 24;
    }

    if (goldNeeded > 0)
    {
        const std::string goldLine = TextFormat("Gold: %d / %d to activate", mGoldCount, goldNeeded);
        DrawText(goldLine.c_str(),
                 static_cast<int>(panel.x + 12.0f),
                 textY,
                 16,
                 Fade(GOLD, 0.95f));
        textY += 24;
    }

    const std::string hint = goldNeeded > 0
        ? "Defeat enemies to collect gold, then touch the table"
        : "Find the table with map";
    DrawText(hint.c_str(),
             static_cast<int>(panel.x + 12.0f),
             textY,
             16,
             Fade(GRAY, 0.9f));
}

void Level1::drawCompassIndicator()
{
    if (!mCompassUI)
    {
        return;
    }
    const bool shouldShow = isCompassSelected() && !mQuestComplete;
    mCompassUI->setIsActive(shouldShow);
    if (shouldShow)
    {
        mCompassUI->render();
    }
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

void Level1::handlePrimaryAttackAction()
{
    if (mIsGameOver)
    {
        return;
    }
    if (!isBranchSelected() || isCompassSelected())
    {
        return;
    }
    tryThrowBranchAtEnemy();
}

void Level1::handleMeleeAttackAction()
{
    if (mIsGameOver)
    {
        return;
    }
    if (!isAxeSelected())
    {
        return;
    }
    tryMeleeAttack();
}

void Level1::onDifficultyPresetChanged(int index)
{
    const int presetCount = branch::DIFFICULTY_PRESET_COUNT;
    if (presetCount <= 0)
    {
        return;
    }

    const int clamped = std::clamp(index, 0, presetCount - 1);
    mDifficulty.index = clamped;
    applyDifficulty(mDifficulty);
    resetBranchInventory();
    spawnQuestTarget();
    if (mCompassUI)
    {
        mCompassUI->setTarget(mTable.get());
    }
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
    mMeleeTimer = 0.0f;
    mIsGameOver = false;
    clearBranches();
    resetBranchInventory();
    clearGoldCoins(true);

    spawnQuestTarget();
    if (mCompassUI)
    {
        mCompassUI->setTarget(mTable.get());
        mCompassUI->setPlayer(mPlayer);
    }

    mSkipPlayerChunkForNextEnemySpawn = true;
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

void Level1::updateTreesForStream()
{
    for (Tree* tree : mActiveTrees)
    {
        if (!tree)
        {
            continue;
        }
        tree->setIsActive(false);
        tree->setCanCollide(false);
        removeCollidableEntity(mCollidableEntities, tree);
    }
    mActiveTrees.clear();

    const int radius = getChunkLoadRadius();
    const int span = getChunkSpan();
    const int minChunkX = mCurrentChunkX - radius;
    const int minChunkY = mCurrentChunkY - radius;

    for (int dy = 0; dy < span; ++dy)
    {
        for (int dx = 0; dx < span; ++dx)
        {
            const std::pair<int, int> chunkKey{
                minChunkX + dx,
                minChunkY + dy
            };

            std::vector<Tree*> &bucket = mChunkTrees[chunkKey];
            if (bucket.empty())
            {
                spawnTreesForChunk(chunkKey, bucket);
            }

            for (Tree* tree : bucket)
            {
                if (!tree || tree->isDead())
                {
                    if (tree)
                    {
                        tree->setIsActive(false);
                        tree->setCanCollide(false);
                        removeCollidableEntity(mCollidableEntities, tree);
                    }
                    continue;
                }

                tree->setIsActive(true);
                tree->setCanCollide(true);
                const bool alreadyPresent = std::find(mCollidableEntities.begin(),
                                                      mCollidableEntities.end(),
                                                      tree) != mCollidableEntities.end();
                if (!alreadyPresent)
                {
                    mCollidableEntities.push_back(tree);
                }
                mActiveTrees.push_back(tree);
            }
        }
    }
}

void Level1::spawnTreesForChunk(const std::pair<int, int> &chunk,
                                std::vector<Tree*> &bucket)
{
    const int chunkStartX = chunk.first * mChunkSize;
    const int chunkStartY = chunk.second * mChunkSize;
    const int spacing = std::max(TREE_SPAWN_SETTINGS.spacing, 1);

    ResourceManager &rm = ResourceManager::instance();
    const std::vector<Rectangle> &treeRects = rm.getSpriteRects(TreeConstants::SPRITE_TAG);
    const int treeVariantCount = std::max(1, static_cast<int>(treeRects.size()));
    if (treeRects.empty())
    {
        LOG_WARNING("Tree sprites missing tag 'TREE' in atlas metadata; using fallback variant.");
    }

    int spawnedCount = 0;

    for (int row = 0; row < mChunkSize; row += spacing)
    {
        for (int col = 0; col < mChunkSize; col += spacing)
        {
            const int worldX = chunkStartX + col;
            const int worldY = chunkStartY + row;

            const float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, TREE_SPAWN_SETTINGS.salt);
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

            const float worldPosX = (static_cast<float>(worldX) + 0.5f) * mTileSize;
            const float worldPosY = (static_cast<float>(worldY) + 0.5f) * mTileSize;

            Tree* tree = new Tree({worldPosX, worldPosY},
                                  treeHeightPx,
                                  treeVariant,
                                  rootHeight,
                                  rootWidthRatio);
            tree->setIsActive(false);
            tree->setCanCollide(true);
            bucket.push_back(tree);
            spawnedCount++;
        }
    }

    if (isDebugMode() && spawnedCount > 0)
    {
        LOG_DEBUG(TextFormat("Chunk (%d,%d) spawned %d trees",
                             chunk.first,
                             chunk.second,
                             spawnedCount));
    }
}

void Level1::generateRocks()
{
    clearRocks();

    const double tGenStart = GetTime();

    ResourceManager &rm = ResourceManager::instance();
    std::vector<Rectangle> rockVariants;
    rockVariants.reserve(16);

    for (const char *tag : ROCK_SPRITE_TAGS)
    {
        const std::vector<Rectangle> &rects = rm.getSpriteRects(tag);
        rockVariants.insert(rockVariants.end(), rects.begin(), rects.end());
    }

    if (rockVariants.empty())
    {
        LOG_WARNING("Rock sprites missing tags SMALLROCK/SMALLROCK2/SMALLROCK3/LARGEROCK; skipping rock generation.");
        return;
    }

    const int spacing = std::max(ROCK_SPAWN_SETTINGS.spacing, 1);
    const int startX = getChunkStartX();
    const int startY = getChunkStartY();

    const float safeRadius = ROCK_SPAWN_SETTINGS.safeRadiusTiles * mTileSize;
    const float safeRadiusSq = safeRadius * safeRadius;
    const Vector2 playerPos = mPlayer ? mPlayer->getPosition() : mPlayerSpawnPosition;

    auto tooCloseToPoint = [safeRadiusSq](const Vector2 &candidate, const Vector2 &other)
    {
        const float dx = candidate.x - other.x;
        const float dy = candidate.y - other.y;
        return (dx * dx + dy * dy) < safeRadiusSq;
    };

    int spawnedCount = 0;

    for (int row = 0; row < mMapRows; row += spacing)
    {
        for (int col = 0; col < mMapColumns; col += spacing)
        {
            const int worldX = startX + col;
            const int worldY = startY + row;

            const float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, ROCK_SPAWN_SETTINGS.salt);
            if (spawnNoise < ROCK_SPAWN_SETTINGS.spawnThreshold)
            {
                continue;
            }

            const float variantNoise = mMapGenerator.whiteNoise(worldX, worldY, ROCK_SPAWN_SETTINGS.salt + 1u);
            const float heightNoise = mMapGenerator.whiteNoise(worldX, worldY, ROCK_SPAWN_SETTINGS.salt + 2u);
            const float colliderNoise = mMapGenerator.whiteNoise(worldX, worldY, ROCK_SPAWN_SETTINGS.salt + 3u);

            size_t variantIndex = static_cast<size_t>(variantNoise * static_cast<float>(rockVariants.size()));
            if (variantIndex >= rockVariants.size())
            {
                variantIndex = rockVariants.size() - 1;
            }

            const float rockHeight = ROCK_SPAWN_SETTINGS.minHeightPx +
                                     heightNoise * (ROCK_SPAWN_SETTINGS.maxHeightPx - ROCK_SPAWN_SETTINGS.minHeightPx);
            const float colliderHeightRatio = ROCK_SPAWN_SETTINGS.minColliderRatio +
                                              colliderNoise * (ROCK_SPAWN_SETTINGS.maxColliderRatio - ROCK_SPAWN_SETTINGS.minColliderRatio);
            const float colliderWidthRatio = 0.65f + colliderNoise * 0.3f;

            const float worldPosX = (static_cast<float>(worldX) + 0.5f) * mTileSize;
            const float worldPosY = (static_cast<float>(worldY) + 0.5f) * mTileSize;
            const Vector2 worldPos{worldPosX, worldPosY};

            if ((safeRadiusSq > 0.0f && (tooCloseToPoint(worldPos, mPlayerSpawnPosition) ||
                                         tooCloseToPoint(worldPos, playerPos))))
            {
                continue;
            }

            Rock *rock = new Rock(worldPos,
                                  rockVariants[variantIndex],
                                  rockHeight,
                                  colliderHeightRatio,
                                  colliderWidthRatio);
            mRocks.push_back(rock);
            mCollidableEntities.push_back(rock);
            spawnedCount++;
        }
    }

    const double tGenEnd = GetTime();
    LOG_DEBUG(TextFormat("Generated %d rocks in %.2fms",
                         spawnedCount,
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
    const bool skipPlayerChunk = mSkipPlayerChunkForNextEnemySpawn;
    const std::pair<int, int> playerChunk{mCurrentChunkX, mCurrentChunkY};
    bool skippedPlayerChunk = false;

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
            if (skipPlayerChunk && chunkKey == playerChunk)
            {
                skippedPlayerChunk = true;
                continue;
            }
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

    if (mEnemies.empty() && !hasStoredEnemies && !(skipPlayerChunk && skippedPlayerChunk))
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

    mSkipPlayerChunkForNextEnemySpawn = false;

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

void Level1::updateBoxesForStream()
{
    for (Box* box : mBoxes)
    {
        if (!box)
        {
            continue;
        }
        removeCollidableEntity(mCollidableEntities, box);
        box->setIsActive(false);
    }
    mBoxes.clear();

    const int radius = getChunkLoadRadius();
    const int span = getChunkSpan();
    const int minChunkX = mCurrentChunkX - radius;
    const int minChunkY = mCurrentChunkY - radius;

    for (int dy = 0; dy < span; ++dy)
    {
        for (int dx = 0; dx < span; ++dx)
        {
            const std::pair<int, int> chunkKey{
                minChunkX + dx,
                minChunkY + dy
            };

            std::vector<Box*> &bucket = mChunkBoxes[chunkKey];
            if (bucket.empty())
            {
                spawnBoxesForChunk(chunkKey, bucket);
            }

            for (Box* box : bucket)
            {
                if (!box || box->isCollected())
                {
                    removeCollidableEntity(mCollidableEntities, box);
                    continue;
                }

                box->setIsActive(true);
                if (std::find(mCollidableEntities.begin(), mCollidableEntities.end(), box) == mCollidableEntities.end())
                {
                    mCollidableEntities.push_back(box);
                }
                mBoxes.push_back(box);
            }
        }
    }
}

void Level1::spawnBoxesForChunk(const std::pair<int, int> &chunk,
                                std::vector<Box*> &bucket)
{
    const int chunkStartX = chunk.first * mChunkSize;
    const int chunkStartY = chunk.second * mChunkSize;
    int spawnedCount = 0;
    const int maxBoxesPerChunk = 3;

    for (int row = 0; row < mChunkSize; row += BOX_SPAWN_SETTINGS.spacing)
    {
        for (int col = 0; col < mChunkSize; col += BOX_SPAWN_SETTINGS.spacing)
        {
            const int worldX = chunkStartX + col;
            const int worldY = chunkStartY + row;
            const float spawnNoise = mMapGenerator.whiteNoise(worldX, worldY, BOX_SPAWN_SETTINGS.salt);
            if (spawnNoise < BOX_SPAWN_SETTINGS.spawnThreshold)
            {
                continue;
            }

            const float worldPosX = (static_cast<float>(worldX) + 0.5f) * mTileSize;
            const float worldPosY = (static_cast<float>(worldY) + 0.5f) * mTileSize;
            Box* box = new Box({worldPosX, worldPosY});
            bucket.push_back(box);
            spawnedCount++;

            if (spawnedCount >= maxBoxesPerChunk)
            {
                break;
            }
        }
        if (spawnedCount >= maxBoxesPerChunk)
        {
            break;
        }
    }

    if (isDebugMode() && spawnedCount > 0)
    {
        LOG_DEBUG(TextFormat("Chunk (%d,%d) spawned %d boxes",
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
        updateTreesForStream();
        generateRocks();
        generateEnemies();
        updateBoxesForStream();
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
