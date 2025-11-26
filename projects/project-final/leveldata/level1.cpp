#include "level1.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include "../lib/ResourceManager.h"
#include "../lib/Inventory.h"
#include "../lib/ui/InventoryBar.h"
#include "../lib/ui/Button.h"
#include "../lib/Profiler.h"
#include "branch.h"
#include "box.h"
#include "compass.h"
#include "goldcoin.h"
#include "rock.h"
#include "table_with_map.h"
#include "LevelSelectScene.h"
#include "level2.h"
#include "ResourceTags.h"
#include "level1_consts.h"
#include "attack_enemy.h"
#include "../lib/SceneController.h"
#include "../lib/Music.h"

extern SceneController* gSceneController;
using namespace level1_consts;

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
        0.9f,
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

    struct ShooterSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
        float minHeightPx = 0.0f;
        float maxHeightPx = 0.0f;
        float maxDistanceScale = 1.0f;
        float shooterChance = 0.35f;
    };

    constexpr ShooterSpawnSettings SHOOTER_SPAWN_SETTINGS{
        0x9e3779b9u,
        0.9f,
        4,
        44.0f,
        68.0f,
        0.9f,
        0.35f
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
    constexpr float BOSS_HEALTH = 32.0f;
    constexpr float BOSS_SUMMON_INTERVAL = 6.0f;
    constexpr int BOSS_MAX_MINIONS = 3;
    constexpr float BOSS_SUMMON_RADIUS = 220.0f;
    constexpr float BOSS_BAR_DISTANCE = 540.0f;
    constexpr float BOSS_HEIGHT = DogConstants::MAX_HEIGHT * 0.8f;
    constexpr float BOSS_INDICATOR_MARGIN = 42.0f;
    constexpr float BOSS_INDICATOR_ARROW_SIZE = 18.0f;

    constexpr float SUMMON_EFFECT_DURATION = 0.9f;
    constexpr float SUMMON_EFFECT_START_RADIUS = 30.0f;
    constexpr float SUMMON_EFFECT_END_RADIUS = 190.0f;
    constexpr Color SUMMON_EFFECT_COLOR = {255, 170, 64, 255};

    constexpr const char *BGM_EXPLORE = "assets/Minifantasy_Dungeon_Music/Music/Goblins_Den_(Regular).wav";
    constexpr const char *BGM_BATTLE = "assets/Minifantasy_Dungeon_Music/Music/Goblins_Dance_(Battle).wav";
    constexpr const char *SFX_CHEST_OPEN[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/01_chest_open_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/01_chest_open_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/01_chest_open_3.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/01_chest_open_4.wav"
    };
    constexpr size_t SFX_CHEST_OPEN_COUNT = sizeof(SFX_CHEST_OPEN) / sizeof(SFX_CHEST_OPEN[0]);
}

void Level1::initialise()
{
    LOG_INFO("Level1 initialised");
    setChunkSize(mChunkSize);
    setChunkLoadRadius(mChunkLoadRadius);
    ensureTreeAtlas();
    ensureTileTexture();
    spawnPlayer();
    mLastPlayerHealth = mPlayer ? mPlayer->getHealth() : PlayerConstants::MAX_HEALTH;
    mHurtOverlayTimer = 0.0f;
    mBossSummonEffects.clear();
    mMeleeEffects.clear();
    ensureHurtShader();
    ensureMusicNotes();
    applyDifficulty(mDifficulty);
    resetBranchInventory();
    mMeleeDamage = combat::MELEE_DAMAGE;
    mPotionCapacity = POTION_CAPACITY_DEFAULT;
    mPotionCount = 0;
    mBranchDamage = branch::PROJECTILE_DAMAGE;
    mSwordUpgradeCount = 0;
    mShurikenUpgradeCount = 0;
    mRecoverableThrows = false;
    mShopOpen = false;
    mShopSuppressed = false;
    mBossSpawned = false;
    mBossDefeated = false;
    mBossSummonTimer = 0.0f;
    mAttackersUnlocked = false;
    mShooterPhaseActive = false;
    mShootersRemaining = 0;
    mShooterSpawnTimer = 0.0f;
    mLastSelectedSlot = -1;
    mToolHintTimer = 0.0f;
    mToolHintText.clear();
    mActiveMusicPath.clear();
    mQuestDescription = "Reach the map table, shop, and defeat the guardian";
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
    mBossAdvanceRequested = false;
    mLastSelectedSlot = -1;
    mToolHintTimer = 0.0f;
    mToolHintText.clear();
    updateChunkStream(true);
    // Lighting shader temporarily disabled; keep call commented for future restoration.
    // initialiseLightingShader();

    mTutorialOverlayVisible = true;
    mTutorialOverlayDismissed = false;
    mTutorialOverlayDisplayTimer = 0.0f;
    mTutorialOverlayFadeTimer = 0.0f;
    mTutorialReopenHintTimer = 0.0f;
    playExplorationMusic();
}

void Level1::update(float deltaTime)
{
    updateShop(deltaTime);
    if (mShopOpen)
    {
        updateHurtOverlay(deltaTime);
        updateMeleeEffects(deltaTime);
        return;
    }

    const bool profile = isDebugMode();
    FrameProfiler profiler(profile);

    advanceDayNightCycle(deltaTime);
    if (profile) profiler.mark("dayNight", "world");

    updateChunkStream();
    if (profile) profiler.mark("chunkStream", "world");

    NavMap::beginFrame();
    if (profile) profiler.mark("navBegin", "nav");

    ensureMusicNotes();
    updateMusicNoteBehaviour();
    handleMouseBranchInput();
    if (profile) profiler.mark("input", "input");

    struct CellRange
    {
        int minX;
        int maxX;
        int minY;
        int maxY;
    };

    const float invCellSize = 1.0f / physics::COLLISION_CELL_SIZE;

    auto computeCellRange = [invCellSize](const Entity *entity, float padding) -> CellRange
    {
        const Vector2 pos = entity->getPosition();
        const Vector2 dims = entity->getColliderDimensions();
        const float halfW = (dims.x * 0.5f) + padding;
        const float halfH = (dims.y * 0.5f) + padding;
        return {
            static_cast<int>(std::floor((pos.x - halfW) * invCellSize)),
            static_cast<int>(std::floor((pos.x + halfW) * invCellSize)),
            static_cast<int>(std::floor((pos.y - halfH) * invCellSize)),
            static_cast<int>(std::floor((pos.y + halfH) * invCellSize))
        };
    };

    // Broadphase: bucket nearby colliders into grid cells to trim pair checks.
    std::unordered_map<std::pair<int, int>, std::vector<size_t>, ChunkKeyHash> collisionGrid;
    collisionGrid.reserve(mCollidableEntities.size() * 2);

    for (size_t i = 0; i < mCollidableEntities.size(); ++i)
    {
        Entity *entity = mCollidableEntities[i];
        if (!entity || !entity->getIsActive() || !entity->getCanCollide())
        {
            continue;
        }

        const CellRange cells = computeCellRange(entity, 0.0f);
        for (int cx = cells.minX; cx <= cells.maxX; ++cx)
        {
            for (int cy = cells.minY; cy <= cells.maxY; ++cy)
            {
                const std::pair<int, int> cell{cx, cy};
                collisionGrid[cell].push_back(i);
            }
        }
    }

    // Track which candidates were already added for the current entity.
    std::vector<unsigned int> visited(mCollidableEntities.size(), 0u);
    unsigned int visitToken = 1u;
    std::vector<Entity*> nearby;
    nearby.reserve(32);

    for (size_t i = 0; i < mCollidableEntities.size(); ++i)
    {
        if (visitToken == 0u)
        {
            std::fill(visited.begin(), visited.end(), 0u);
            visitToken = 1u;
        }

        Entity *entity = mCollidableEntities[i];
        if (!entity)
        {
            continue;
        }

        nearby.clear();
        const CellRange cells = computeCellRange(entity, physics::COLLISION_QUERY_MARGIN);
        for (int cx = cells.minX; cx <= cells.maxX; ++cx)
        {
            for (int cy = cells.minY; cy <= cells.maxY; ++cy)
            {
                const std::pair<int, int> cell{cx, cy};
                auto it = collisionGrid.find(cell);
                if (it == collisionGrid.end())
                {
                    continue;
                }

                for (size_t candidateIdx : it->second)
                {
                    if (candidateIdx == i || visited[candidateIdx] == visitToken)
                    {
                        continue;
                    }
                    Entity *candidate = mCollidableEntities[candidateIdx];
                    if (!candidate || !candidate->getIsActive() || !candidate->getCanCollide())
                    {
                        continue;
                    }
                    visited[candidateIdx] = visitToken;
                    nearby.push_back(candidate);
                }
            }
        }
        ++visitToken;
        entity->update(deltaTime, mPlayer, mMap, nearby);
    }
    if (profile) profiler.mark("entities", "entities");

    resolveBranchImpacts();
    updateBranchPickups();
    cleanupBranches();
    cleanupInactiveTrees();
    if (mNavStaticsDirty)
    {
        refreshNavMeshStatics();
    }
    if (profile) profiler.mark("cleanup", "world");

    updateBoxRewards();
    updateGoldCoins();
    updatePlayerAttack(deltaTime);
    updateMeleeTimer(deltaTime);
    updateMeleeEffects(deltaTime);
    updateBossFight(deltaTime);
    updateBossSummonEffects(deltaTime);
    updateSpreadProjectiles(deltaTime);
    updatePostBossShooters(deltaTime);
    updateHurtOverlay(deltaTime);
    updateMinimapTexture();
    if (profile) profiler.mark("combat", "combat");

    updateCameraFromPlayer(deltaTime);
    updateGameOverState();
    updateInventoryUI(deltaTime);
    updateTutorialOverlay(deltaTime);
    if (mCompassUI)
    {
        mCompassUI->update(deltaTime, mPlayer, mMap, mCollidableEntities);
    }
    updateQuestState();
    if (profile) profiler.mark("uiQuest", "ui");

    profiler.logSummary();
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
        mLastPlayerHealth = mPlayer->getHealth();
        mHurtOverlayTimer = 0.0f;
        mIsGameOver = false;
        mCamera.target = mPlayer->getPosition();
        return;
    }

    mPlayer = new Player(c::ORIGIN, {54.0f, 75.0f});
    updatePlayerSpawnPoint(mPlayer->getPosition());
    mPlayer->restoreFullHealth();
    mLastPlayerHealth = mPlayer->getHealth();
    mHurtOverlayTimer = 0.0f;
    mMeleeEffects.clear();
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
    auto applyObstructionFade = [this](Entity *entity)
    {
        if (!entity || !mPlayer)
        {
            return;
        }
        const bool isTree = dynamic_cast<Tree*>(entity) != nullptr;
        const bool isRock = dynamic_cast<Rock*>(entity) != nullptr;
        if (!isTree && !isRock)
        {
            return;
        }

        const Vector2 playerPos = mPlayer->getPosition();
        const Vector2 entityPos = entity->getPosition();
        const float dx = entityPos.x - playerPos.x;
        const float dy = entityPos.y - playerPos.y;
        const Vector2 playerCollider = mPlayer->getColliderDimensions();
        const Vector2 entityCollider = entity->getColliderDimensions();
        const Vector2 entityScale = entity->getScale();
        const float playerRadius = std::max(playerCollider.x, playerCollider.y) * 0.5f;
        const float colliderRadius = std::max(entityCollider.x, entityCollider.y) * 0.5f;
        const float visualRadius = std::max(entityScale.x, entityScale.y) * (isTree ? 0.4f : 0.3f);
        const float radius = playerRadius + std::max(colliderRadius, visualRadius);
        const float distSq = dx * dx + dy * dy;
        const bool obstructing = distSq <= radius * radius;
        const float alpha = obstructing ? 0.2f : 1.0f;
        entity->setTint(Fade(WHITE, alpha));
    };

    for (Entity* entity : mCollidableEntities)
    {
        applyObstructionFade(entity);
        entity->render();
    }
    drawMeleeEffects();
    drawBossSummonEffects();
    drawSpreadProjectiles();

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
    drawToolHint();
    drawCompassIndicator();
    drawQuestLog();
    drawTutorialOverlay();
    drawGameOverOverlay();
    drawBossBar();
    drawBossDirectionIndicator();
    drawShopOverlay();
    drawMapTableUI();
    drawMinimap();
    drawHurtOverlay();
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
    mBossSummonEffects.clear();
    mMeleeEffects.clear();
    mHurtShader.unload();
    mHurtShaderReady = false;
    mHurtOverlayTimer = 0.0f;
    mTutorialReopenHintTimer = 0.0f;
    mCollidableEntities.clear();
    mBossAdvanceRequested = false;
    mAttackersUnlocked = false;
    mLastSelectedSlot = -1;
    mToolHintTimer = 0.0f;
    mToolHintText.clear();
    mActiveMusicPath.clear();
    AudioManager::stopBGM();
    if (mMinimapReady)
    {
        UnloadRenderTexture(mMinimap);
        mMinimapReady = false;
    }

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
    mNavStaticsDirty = true;
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
    if (!mRocks.empty())
    {
        mNavStaticsDirty = true;
    }
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
    if (mBoss)
    {
        removeCollidableEntity(mCollidableEntities, mBoss);
        delete mBoss;
        mBoss = nullptr;
    }
    for (Dog* minion : mBossMinions)
    {
        if (!minion)
        {
            continue;
        }
        removeCollidableEntity(mCollidableEntities, minion);
        delete minion;
    }
    mBossMinions.clear();
    mBossSpawned = false;
    mBossDefeated = false;
    mBossSummonEffects.clear();
    mMeleeEffects.clear();
    mEnemies.clear();
}

void Level1::clearMusicNotes()
{
    destroyOwnedEntities(mMusicNotes, mCollidableEntities);
}

void Level1::cleanupInactiveTrees()
{
    bool removed = false;
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
            removed = true;
        }
        else
        {
            ++it;
        }
    }

    if (removed)
    {
        mNavStaticsDirty = true;
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

    Branch *projectile = new Branch(start,
                                    direction,
                                    travelDistance,
                                    branch::THROW_SPEED,
                                    mBranchDamage,
                                    mRecoverableThrows);
    projectile->setRecoverable(mRecoverableThrows);
    projectile->setUseShuriken(mRecoverableThrows);
    mBranches.push_back(projectile);
    mCollidableEntities.push_back(projectile);
    playThrowSFX();
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
                playMeleeHitSFX();
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
        if (!projectile)
        {
            removeCollidableEntity(mCollidableEntities, projectile);
            delete projectile;
            it = mBranches.erase(it);
            continue;
        }

        if (projectile->isRecoverable() && projectile->isSpent() && !projectile->isCollected())
        {
            ++it;
            continue;
        }

        if (projectile->isCollected() || (!projectile->isRecoverable() && (projectile->isSpent() || !projectile->getIsActive())))
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

void Level1::addPotions(int amount)
{
    if (amount <= 0)
    {
        return;
    }
    mPotionCapacity = std::max(POTION_CAPACITY_DEFAULT, mPotionCapacity);
    mPotionCount = std::clamp(mPotionCount + amount, 0, mPotionCapacity);
    syncPotionSlot();
}

void Level1::applyDifficulty(const DifficultyState &state)
{
    mDifficulty = state;
    mInitialBranchCount = std::clamp(mDifficulty.initialBranches(), 0, branch::MAX_HELD);
    mBoxBranchReward = std::max(1, mDifficulty.boxReward());
    mPotionCapacity = POTION_CAPACITY_DEFAULT;
}

bool Level1::usePotion()
{
    if (!mPlayer || mPotionCount <= 0)
    {
        return false;
    }
    if (mPlayer->getHealth() >= mPlayer->getMaxHealth() - 0.01f)
    {
        return false;
    }
    --mPotionCount;
    mPlayer->heal(POTION_HEAL_AMOUNT);
    syncPotionSlot();
    syncGoldSlot();
    playPotionSFX();
    LOG_INFO(TextFormat("Potion used: +%.1f HP (potions left=%d)", POTION_HEAL_AMOUNT, mPotionCount));
    return true;
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

void Level1::updateBranchPickups()
{
    if (!mPlayer)
    {
        return;
    }

    auto it = mBranches.begin();
    while (it != mBranches.end())
    {
        Branch *branch = *it;
        if (!branch)
        {
            it = mBranches.erase(it);
            continue;
        }

        if (branch->isRecoverable() && branch->isSpent() && !branch->isCollected())
        {
            if (mPlayer->intersects(*branch))
            {
                branch->markCollected();
                removeCollidableEntity(mCollidableEntities, branch);
                mBranchInventory = std::clamp(mBranchInventory + 1, 0, mBranchCapacity);
                syncBranchSlot();
                playGoldPickupSFX();
            }
            ++it;
            continue;
        }

        ++it;
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
    playGoldPickupSFX();
    ++mGoldCount;
    syncGoldSlot();
}

void Level1::handleEnemyDefeated(Enemy *enemy)
{
    if (!enemy)
    {
        return;
    }

    if (enemy == mBoss)
    {
        mBossDefeated = true;
    }

    playEnemyDeathSFX();

    const float dropRoll = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f;
    const float dropChance = std::clamp(enemyGoldDropChance(), 0.0f, 1.0f);
    if (dropRoll > dropChance)
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

    if (enemy == mBoss)
    {
        handleBossDefeated();
    }
}

void Level1::handleBossDefeated()
{
    if (mBossAdvanceRequested)
    {
        return;
    }
    mBossAdvanceRequested = true;
    mAttackersUnlocked = true;
    mShooterPhaseActive = true;
    mShootersRemaining = 5;
    mShooterSpawnTimer = 0.1f;
    playBattleMusic();
}

std::unique_ptr<Scene> Level1::createNextSceneAfterBoss()
{
    return nullptr;
}

float Level1::enemyGoldDropChance() const
{
    return GOLD_DROP_CHANCE;
}

void Level1::updatePostBossShooters(float deltaTime)
{
    if (!mShooterPhaseActive || !mPlayer)
    {
        return;
    }
    mShooterSpawnTimer = std::max(0.0f, mShooterSpawnTimer - deltaTime);

    int activeShooters = 0;
    for (Enemy *enemy : mEnemies)
    {
        if (dynamic_cast<AttackEnemy*>(enemy) && enemy->getIsActive() && !enemy->isDead())
        {
            ++activeShooters;
        }
    }

    if (mShootersRemaining <= 0 && activeShooters == 0)
    {
        mShooterPhaseActive = false;
        playExplorationMusic();
        return;
    }

    const int maxActive = 3;
    if (mShootersRemaining > 0 && activeShooters < maxActive && mShooterSpawnTimer <= 0.0f)
    {
        spawnShooterEnemy();
        --mShootersRemaining;
        mShooterSpawnTimer = 1.75f;
    }
}

void Level1::spawnShooterEnemy()
{
    if (!mPlayer)
    {
        return;
    }

    const float spawnRadius = std::max(static_cast<float>(c::SCREEN_WIDTH), static_cast<float>(c::SCREEN_HEIGHT)) * 0.35f;
    const float angle = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f * 2.0f * PI;
    const Vector2 playerPos = mPlayer->getPosition();
    Vector2 pos = {
        playerPos.x + cosf(angle) * spawnRadius,
        playerPos.y + sinf(angle) * spawnRadius
    };

    const int variant = std::clamp(GetRandomValue(0, 2), 0, 2);
    AttackEnemy *attacker = new AttackEnemy(pos,
                                            variant,
                                            &mSpreadProjectiles,
                                            42.0f);
    attacker->setNavMap(&mNavMap);
    mEnemies.push_back(attacker);
    mCollidableEntities.push_back(attacker);
}

void Level1::ensureMinimap()
{
    const int width = mMapColumns;
    const int height = mMapRows;
    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (mMinimapReady)
    {
        if (mMinimap.texture.width != width || mMinimap.texture.height != height)
        {
            UnloadRenderTexture(mMinimap);
            mMinimapReady = false;
        }
    }

    if (!mMinimapReady)
    {
        mMinimap = LoadRenderTexture(width, height);
        mMinimapReady = mMinimap.id != 0;
    }
}

void Level1::playExplorationMusic()
{
    if (mActiveMusicPath == BGM_EXPLORE)
    {
        return;
    }
    AudioManager::playBGM(BGM_EXPLORE, true);
    mActiveMusicPath = BGM_EXPLORE;
}

void Level1::playBattleMusic()
{
    if (mActiveMusicPath == BGM_BATTLE)
    {
        return;
    }
    AudioManager::playBGM(BGM_BATTLE, true);
    mActiveMusicPath = BGM_BATTLE;
}

void Level1::updateMinimapTexture()
{
    if (mLevelData.empty() || mMapColumns <= 0 || mMapRows <= 0)
    {
        return;
    }

    ensureMinimap();
    if (!mMinimapReady)
    {
        return;
    }

    BeginTextureMode(mMinimap);
    ClearBackground({12, 18, 12, 220});

    // Base tiles
    for (int y = 0; y < mMapRows; ++y)
    {
        for (int x = 0; x < mMapColumns; ++x)
        {
            const size_t idx = static_cast<size_t>(y * mMapColumns + x);
            const unsigned int tile = (idx < mLevelData.size()) ? mLevelData[idx] : 0u;
            const Color color = tile == 0 ? Color{20, 40, 24, 230} : Color{38, 92, 46, 230};
            DrawPixel(x, y, color);
        }
    }

    auto plot = [this](const Vector2 &worldPos, Color color, int size = 1)
    {
        const int gx = static_cast<int>(std::floor(worldPos.x / mTileSize)) - getChunkStartX();
        const int gy = static_cast<int>(std::floor(worldPos.y / mTileSize)) - getChunkStartY();
        if (gx < 0 || gx >= mMapColumns || gy < 0 || gy >= mMapRows)
        {
            return;
        }
        const int half = std::max(0, size / 2);
        for (int dy = -half; dy <= half; ++dy)
        {
            for (int dx = -half; dx <= half; ++dx)
            {
                const int px = gx + dx;
                const int py = gy + dy;
                if (px >= 0 && px < mMapColumns && py >= 0 && py < mMapRows)
                {
                    DrawPixel(px, py, color);
                }
            }
        }
    };

    // Collidable entities
    for (Entity *entity : mCollidableEntities)
    {
        if (!entity || !entity->getIsActive())
        {
            continue;
        }
        const int markerSize = 2;
        plot(entity->getPosition(), Color{120, 120, 120, 255}, markerSize);
    }

    // Enemies
    for (Enemy *enemy : mEnemies)
    {
        if (!enemy || !enemy->getIsActive() || enemy->isDead())
        {
            continue;
        }
        plot(enemy->getPosition(), RED, 3);
    }

    // Player
    if (mPlayer && mPlayer->getIsActive())
    {
        plot(mPlayer->getPosition(), SKYBLUE, 4);
    }

    // POI: map table, boss
    if (mTable && mTable->getIsActive())
    {
        plot(mTable->getPosition(), PURPLE, 3);
    }
    if (mBoss && mBoss->getIsActive())
    {
        plot(mBoss->getPosition(), Color{200, 40, 200, 255}, 4);
    }

    EndTextureMode();
}

void Level1::drawMinimap() const
{
    if (!mMinimapReady)
    {
        return;
    }

    const float maxSize = 220.0f;
    const float viewTilesX = std::min(static_cast<float>(mChunkSize * 2), static_cast<float>(mMapColumns));
    const float viewTilesY = std::min(static_cast<float>(mChunkSize * 2), static_cast<float>(mMapRows));
    const float pixelsPerTile = maxSize / std::max(std::max(viewTilesX, viewTilesY), 1.0f);
    const float destW = viewTilesX * pixelsPerTile;
    const float destH = viewTilesY * pixelsPerTile;

    const float margin = 12.0f;
    const Rectangle dest = {
        static_cast<float>(c::SCREEN_WIDTH) - destW - margin,
        margin,
        destW,
        destH
    };

    // Center view around player (fallback to camera target).
    const Vector2 focusWorld = mPlayer ? mPlayer->getPosition() : mCamera.target;
    const int focusTileX = static_cast<int>(std::floor(focusWorld.x / mTileSize)) - getChunkStartX();
    const int focusTileY = static_cast<int>(std::floor(focusWorld.y / mTileSize)) - getChunkStartY();
    const float halfViewX = viewTilesX * 0.5f;
    const float halfViewY = viewTilesY * 0.5f;
    const float srcW = viewTilesX;
    const float srcH = viewTilesY;
    const float maxSrcX = std::max(0.0f, static_cast<float>(mMapColumns) - srcW);
    const float maxSrcY = std::max(0.0f, static_cast<float>(mMapRows) - srcH);
    const float srcX = std::clamp(focusTileX - halfViewX, 0.0f, maxSrcX);
    const float srcY = std::clamp(focusTileY - halfViewY, 0.0f, maxSrcY);
    // Render textures are stored Y-flipped in Raylib; anchor from the top by offsetting from texture height and flip height.
    const float texH = std::fabs(static_cast<float>(mMinimap.texture.height));
    Rectangle src = {
        srcX,
        texH + 60 - srcY, // why 60? I have no idea, but it just make the player center, and I have no f*** why.
        srcW,
        -srcH
    };

    DrawRectangleRec({dest.x - 4.0f, dest.y - 4.0f, dest.width + 8.0f, dest.height + 8.0f},
                     Fade(BLACK, 0.6f));
    DrawTexturePro(mMinimap.texture, src, dest, {0.0f, 0.0f}, 0.0f, WHITE);
    DrawRectangleLinesEx(dest, 2.0f, Fade(RAYWHITE, 0.8f));
}
void Level1::spawnBoss()
{
    if (!mTable || mBoss)
    {
        return;
    }

    Vector2 pos = mTable->getPosition();
    // Spawn offscreen relative to player and let boss walk in.
    if (mPlayer)
    {
        const Vector2 playerPos = mPlayer->getPosition();
        const float angle = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f * 2.0f * PI;
        const float distance = std::max(static_cast<float>(c::SCREEN_WIDTH), static_cast<float>(c::SCREEN_HEIGHT)) * 0.75f;
        pos = {
            playerPos.x + cosf(angle) * distance,
            playerPos.y + sinf(angle) * distance
        };
    }

    Dog* boss = new Dog(pos, DogConstants::STANDING_VARIANT, BOSS_HEIGHT);
    boss->setNavMap(&mNavMap);
    boss->setBaseMoveSpeed(DogConstants::CHASE_SPEED * 0.6f);
    boss->setPatrolSpeed(DogConstants::PATROL_SPEED * 0.6f);
    boss->setChaseSpeed(DogConstants::CHASE_SPEED * 0.75f);
    boss->setBaseDetectionRadius(DogConstants::DETECTION_RADIUS * 1.2f);
    boss->setTextureFacesLeft(false);
    boss->setIsHorizontalFlipped(false);
    boss->setMaxHealth(BOSS_HEALTH);
    boss->setHealth(BOSS_HEALTH);

    mBoss = boss;
    mBossSpawned = true;
    mBossDefeated = false;
    mBossSummonTimer = BOSS_SUMMON_INTERVAL * 0.5f;
    mBossRepathTimer = 0.1f;
    mShooterPhaseActive = false;
    if (!mBossAdvanceRequested)
    {
        mAttackersUnlocked = false;
    }
    mShootersRemaining = 0;
    mShooterSpawnTimer = 0.0f;
    playBattleMusic();

    mEnemies.push_back(boss);
    mCollidableEntities.push_back(boss);

    LOG_INFO(TextFormat("Boss spawned at (%.1f, %.1f)", pos.x, pos.y));
}

void Level1::spawnBossMinion()
{
    if (!mBoss || mBossDefeated)
    {
        return;
    }

    const float noise = static_cast<float>(GetRandomValue(0, 1000)) / 1000.0f;
    const float angle = noise * 2.0f * PI;
    Vector2 pos = {
        mBoss->getPosition().x + cosf(angle) * BOSS_SUMMON_RADIUS,
        mBoss->getPosition().y + sinf(angle) * BOSS_SUMMON_RADIUS
    };

    const int variantMax = std::max(0, DogConstants::VARIANT_COUNT - 1);
    const int variant = (variantMax > 0) ? GetRandomValue(0, variantMax) : 0;

    Dog* dog = new Dog(pos, variant, DogConstants::DEFAULT_HEIGHT);
    dog->setNavMap(&mNavMap);
    mBossMinions.push_back(dog);
    mEnemies.push_back(dog);
    mCollidableEntities.push_back(dog);
    mBossSummonEffects.push_back({
        mBoss->getPosition(),
        0.0f,
        SUMMON_EFFECT_DURATION,
        SUMMON_EFFECT_START_RADIUS,
        SUMMON_EFFECT_END_RADIUS
    });
    mBossSummonEffects.push_back({
        pos,
        0.0f,
        SUMMON_EFFECT_DURATION * 0.8f,
        SUMMON_EFFECT_START_RADIUS * 0.6f,
        SUMMON_EFFECT_END_RADIUS * 0.65f
    });

    LOG_INFO(TextFormat("Boss summoned dog[%p] at (%.1f, %.1f)", dog, pos.x, pos.y));
}

void Level1::pruneEnemyList(Enemy *enemy)
{
    if (!enemy)
    {
        return;
    }
    auto it = std::remove(mEnemies.begin(), mEnemies.end(), enemy);
    if (it != mEnemies.end())
    {
        mEnemies.erase(it, mEnemies.end());
    }
}

void Level1::cleanupBossMinions()
{
    auto it = mBossMinions.begin();
    while (it != mBossMinions.end())
    {
        Dog* dog = *it;
        if (!dog || dog->isDead() || !dog->getIsActive())
        {
            if (dog)
            {
                removeCollidableEntity(mCollidableEntities, dog);
                pruneEnemyList(dog);
                delete dog;
            }
            it = mBossMinions.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::updateBossFight(float deltaTime)
{
    cleanupBossMinions();

    if (!mBoss)
    {
        return;
    }

    if (mBoss->isDead() || !mBoss->getIsActive())
    {
        removeCollidableEntity(mCollidableEntities, mBoss);
        pruneEnemyList(mBoss);
        delete mBoss;
        mBoss = nullptr;
        mBossDefeated = true;
        handleBossDefeated();
        return;
    }

    mBossSummonTimer -= deltaTime;
    mBossRepathTimer -= deltaTime;

    if (mBossRepathTimer <= 0.0f && mPlayer)
    {
        if (Dog *bossDog = dynamic_cast<Dog*>(mBoss))
        {
            bossDog->setBaseDetectionRadius(5000.0f);
            bossDog->setBaseMoveSpeed(DogConstants::CHASE_SPEED * 0.9f);
            bossDog->setChaseSpeed(DogConstants::CHASE_SPEED * 1.05f);
            Vector2 toPlayer = {
                mPlayer->getPosition().x - bossDog->getPosition().x,
                mPlayer->getPosition().y - bossDog->getPosition().y
            };
            const float dist = std::max(Vector2Length(toPlayer), 0.001f);
            toPlayer.x /= dist;
            toPlayer.y /= dist;
            bossDog->setVelocity({toPlayer.x * bossDog->getChaseSpeedValue(),
                                  toPlayer.y * bossDog->getChaseSpeedValue()});
        }
        mBossRepathTimer = 0.35f;
    }

    if (mBossSummonTimer <= 0.0f)
    {
        if (mBossMinions.size() < static_cast<size_t>(BOSS_MAX_MINIONS))
        {
            spawnBossMinion();
        }
        mBossSummonTimer = BOSS_SUMMON_INTERVAL;
    }
}

void Level1::updateBossSummonEffects(float deltaTime)
{
    auto it = mBossSummonEffects.begin();
    while (it != mBossSummonEffects.end())
    {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration)
        {
            it = mBossSummonEffects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level1::drawBossSummonEffects()
{
    if (mBossSummonEffects.empty())
    {
        return;
    }

    for (const SummonEffect &fx : mBossSummonEffects)
    {
        const float progress = std::clamp(fx.elapsed / std::max(fx.duration, 0.0001f), 0.0f, 1.0f);
        const float eased = progress * (2.0f - progress);
        const float radius = fx.startRadius + (fx.endRadius - fx.startRadius) * eased;
        const float alpha = std::clamp(1.0f - eased, 0.0f, 1.0f);
        const Color outline = Fade(SUMMON_EFFECT_COLOR, alpha * 0.9f);
        const Color fill = Fade(SUMMON_EFFECT_COLOR, alpha * 0.25f);

        DrawCircleV(fx.origin, radius * 0.65f, fill);
        DrawCircleLinesV(fx.origin, radius, outline);
    }
}

void Level1::drawBossBar() const
{
    if (!mBoss || !mPlayer)
    {
        return;
    }
    if (!mBoss->getIsActive() || mBossDefeated)
    {
        return;
    }

    const float distance = Vector2Distance(mBoss->getPosition(), mPlayer->getPosition());
    if (distance > BOSS_BAR_DISTANCE)
    {
        return;
    }

    const float maxHealth = std::max(mBoss->getMaxHealth(), 0.01f);
    const float ratio = std::clamp(mBoss->getHealth() / maxHealth, 0.0f, 1.0f);

    const float barWidth = static_cast<float>(c::SCREEN_WIDTH) * 0.6f;
    const float barHeight = 18.0f;
    Rectangle panel = {
        (c::SCREEN_WIDTH - barWidth) * 0.5f,
        12.0f,
        barWidth,
        barHeight
    };
    Rectangle fill = panel;
    fill.width = panel.width * ratio;

    DrawRectangleRounded(panel, 0.35f, 8, Fade(BLACK, 0.65f));
    DrawRectangleRec(fill, Fade(RED, 0.85f));
    DrawRectangleLinesEx(panel, 2.0f, Fade(RAYWHITE, 0.9f));

    const char *label = "Alpha Dog (Boss)";
    const int fontSize = 18;
    const int labelWidth = MeasureText(label, fontSize);
    DrawText(label,
             static_cast<int>(panel.x + (panel.width - labelWidth) * 0.5f),
             static_cast<int>(panel.y + panel.height + 6.0f),
             fontSize,
             RAYWHITE);
}

void Level1::drawBossDirectionIndicator() const
{
    if (!mBoss || !mBoss->getIsActive() || mBossDefeated)
    {
        return;
    }

    Vector2 bossScreen = GetWorldToScreen2D(mBoss->getPosition(), mCamera);
    const float minX = BOSS_INDICATOR_MARGIN;
    const float maxX = static_cast<float>(c::SCREEN_WIDTH) - BOSS_INDICATOR_MARGIN;
    const float minY = BOSS_INDICATOR_MARGIN;
    const float maxY = static_cast<float>(c::SCREEN_HEIGHT) - BOSS_INDICATOR_MARGIN;

    if (bossScreen.x >= minX && bossScreen.x <= maxX &&
        bossScreen.y >= minY && bossScreen.y <= maxY)
    {
        return;
    }

    Vector2 screenCenter = { static_cast<float>(c::SCREEN_WIDTH) * 0.5f,
                             static_cast<float>(c::SCREEN_HEIGHT) * 0.5f };
    Vector2 direction = {
        bossScreen.x - screenCenter.x,
        bossScreen.y - screenCenter.y
    };
    const float magnitude = Vector2Length(direction);
    if (magnitude < 0.001f)
    {
        return;
    }
    direction.x /= magnitude;
    direction.y /= magnitude;

    Vector2 clampedPos = {
        std::clamp(bossScreen.x, minX, maxX),
        std::clamp(bossScreen.y, minY, maxY)
    };

    const Vector2 tip = {
        clampedPos.x + direction.x * BOSS_INDICATOR_ARROW_SIZE,
        clampedPos.y + direction.y * BOSS_INDICATOR_ARROW_SIZE
    };
    const Vector2 base = {
        clampedPos.x - direction.x * (BOSS_INDICATOR_ARROW_SIZE * 0.5f),
        clampedPos.y - direction.y * (BOSS_INDICATOR_ARROW_SIZE * 0.5f)
    };
    const Vector2 perpendicular = { -direction.y, direction.x };
    const Vector2 leftWing = {
        base.x + perpendicular.x * (BOSS_INDICATOR_ARROW_SIZE * 0.55f),
        base.y + perpendicular.y * (BOSS_INDICATOR_ARROW_SIZE * 0.55f)
    };
    const Vector2 rightWing = {
        base.x - perpendicular.x * (BOSS_INDICATOR_ARROW_SIZE * 0.55f),
        base.y - perpendicular.y * (BOSS_INDICATOR_ARROW_SIZE * 0.55f)
    };

    DrawCircleV(clampedPos, 9.0f, Fade(BLACK, 0.35f));
    DrawTriangle(tip, leftWing, rightWing, Fade(RED, 0.95f));
    DrawTriangleLines(tip, leftWing, rightWing, Fade(RAYWHITE, 0.8f));
    const char *label = "Boss";
    const int fontSize = 16;
    DrawText(label,
             static_cast<int>(clampedPos.x + direction.x * (BOSS_INDICATOR_ARROW_SIZE + 6.0f)),
             static_cast<int>(clampedPos.y + direction.y * (BOSS_INDICATOR_ARROW_SIZE + 6.0f)),
             fontSize,
             RAYWHITE);
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

    playSwingSFX();
    applyMeleeDamage(target);
    mMeleeTimer = std::max(mMeleeCooldown, 0.05f);
    spawnMeleeEffect();
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

    playMeleeHitSFX();
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
    if (SFX_CHEST_OPEN_COUNT > 0)
    {
        const int sfxIndex = GetRandomValue(0, static_cast<int>(SFX_CHEST_OPEN_COUNT) - 1);
        AudioManager::playSFX(SFX_CHEST_OPEN[sfxIndex]);
    }

    if (isDebugMode())
    {
        LOG_INFO(TextFormat("Box[%p] collected. +%d branches (total=%d)",
                            box,
                            mBoxBranchReward,
                            mBranchInventory));
    }
}

void Level1::initialiseInventoryUI()
{
    const size_t slotCount = 5;
    mInventory = std::make_unique<Inventory>(slotCount);
    mAxeSlotIndex = 0;
    mCompassSlotIndex = 1;
    mBranchSlotIndex = 2;
    mGoldSlotIndex = 3;
    mPotionSlotIndex = 4;
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

    InventorySlot potionSlot;
    potionSlot.id = "potion";
    potionSlot.label = "Potion";
    potionSlot.iconTint = Fade(GREEN, 0.9f);
    potionSlot.quantity = mPotionCount;
    applyIcon(potionSlot, tags::POTION);
    mInventory->setSlot(mPotionSlotIndex, potionSlot);

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
    syncWeaponSlot();
    syncPotionSlot();
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
    slot.id = mRecoverableThrows ? "shuriken" : "branches";
    std::string label = mRecoverableThrows ? "Shuriken" : "Branches";
    if (mShurikenUpgradeCount > 0)
    {
        label += " +" + std::to_string(mShurikenUpgradeCount);
    }
    slot.label = label;
    slot.iconTint = mRecoverableThrows ? ORANGE : DARKGREEN;
    slot.quantity = mBranchInventory;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (atlas)
    {
        const char *tag = mRecoverableThrows ? tags::SHURIKEN : BRANCH_SLOT_ICON_TAG;
        Rectangle iconRect = rm.getSpriteRect(tag);
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

void Level1::syncWeaponSlot()
{
    if (!mInventory)
    {
        return;
    }

    if (mInventory->getSlotCount() == 0)
    {
        return;
    }

    if (mAxeSlotIndex >= mInventory->getSlotCount())
    {
        mAxeSlotIndex = 0;
    }

    InventorySlot slot = mInventory->getSlot(mAxeSlotIndex);
    slot.id = "sword";
    std::string label = "Sword";
    if (mSwordUpgradeCount > 0)
    {
        label += " +" + std::to_string(mSwordUpgradeCount);
    }
    slot.label = label;
    slot.iconTint = Fade(WHITE, 0.9f);
    slot.quantity = 1;

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (atlas)
    {
        Rectangle iconRect = rm.getSpriteRect(tags::AXE);
        if (iconRect.width > 0.0f && iconRect.height > 0.0f)
        {
            slot.icon = atlas;
            slot.iconSource = iconRect;
        }
    }

    mInventory->setSlot(mAxeSlotIndex, slot);
}

void Level1::syncPotionSlot()
{
    if (!mInventory)
    {
        return;
    }
    if (mInventory->getSlotCount() == 0)
    {
        return;
    }
    if (mPotionSlotIndex >= mInventory->getSlotCount())
    {
        mPotionSlotIndex = 0;
    }

    InventorySlot slot = mInventory->getSlot(mPotionSlotIndex);
    slot.id = "potion";
    slot.label = "Potion";
    slot.quantity = mPotionCount;
    slot.iconTint = Fade(GREEN, 0.9f);

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (atlas)
    {
        Rectangle iconRect = rm.getSpriteRect(tags::POTION);
        if (iconRect.width > 0.0f && iconRect.height > 0.0f)
        {
            slot.icon = atlas;
            slot.iconSource = iconRect;
        }
    }

    mInventory->setSlot(mPotionSlotIndex, slot);
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

bool Level1::isPotionSelected() const
{
    return mInventory && mInventory->getSelectedIndex() == mPotionSlotIndex;
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

    if ((mBoss && mBoss->getIsActive()) || (mBossSpawned && !mBossDefeated))
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

bool Level1::isPlayerNearTable(float radius) const
{
    if (!mPlayer || !mTable)
    {
        return false;
    }
    const float distance = Vector2Distance(mPlayer->getPosition(), mTable->getPosition());
    return distance <= radius;
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
    if (isCompassSelected())
    {
        return;
    }

    if (isPotionSelected())
    {
        if (usePotion())
        {
            return;
        }
    }

    if (isAxeSelected())
    {
        tryMeleeAttack();
        return;
    }

    if (isBranchSelected())
    {
        tryThrowBranchAtEnemy();
    }
}

void Level1::handleMeleeAttackAction()
{
    handlePrimaryAttackAction();
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
    mLastPlayerHealth = mPlayer->getHealth();
    mHurtOverlayTimer = 0.0f;
    mTutorialReopenHintTimer = 0.0f;
    mMeleeEffects.clear();
    mAttackTimer = 0.0f;
    mMeleeTimer = 0.0f;
    mIsGameOver = false;
    clearBranches();
    resetBranchInventory();
    mPotionCapacity = POTION_CAPACITY_DEFAULT;
    mPotionCount = 0;
    syncPotionSlot();
    clearGoldCoins(true);
    if (mBoss)
    {
        removeCollidableEntity(mCollidableEntities, mBoss);
        delete mBoss;
        mBoss = nullptr;
    }
    for (Dog* minion : mBossMinions)
    {
        if (!minion)
        {
            continue;
        }
        removeCollidableEntity(mCollidableEntities, minion);
        delete minion;
    }
    mBossMinions.clear();
    mBossSpawned = false;
    mBossDefeated = false;
    mBossSummonTimer = 0.0f;
    mBossSummonEffects.clear();
    mSwordUpgradeCount = 0;
    mShurikenUpgradeCount = 0;
    mMeleeDamage = combat::MELEE_DAMAGE;
    mBranchDamage = branch::PROJECTILE_DAMAGE;
    mRecoverableThrows = false;
    mShopOpen = false;
    mShopSuppressed = false;
    mBossAdvanceRequested = false;
    mShooterPhaseActive = false;
    mAttackersUnlocked = false;
    mShootersRemaining = 0;
    mShooterSpawnTimer = 0.0f;
    mLastSelectedSlot = -1;
    mToolHintTimer = 0.0f;
    mToolHintText.clear();
    mActiveMusicPath.clear();
    if (mMinimapReady)
    {
        UnloadRenderTexture(mMinimap);
        mMinimapReady = false;
    }
    playExplorationMusic();
    syncWeaponSlot();

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
    mNavStaticsDirty = false;
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
    mMinimapReady = false;
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

void Level1::refreshNavMeshStatics()
{
    mNavMap.build(mLevelData.data(),
                  mMapColumns,
                  mMapRows,
                  mTileSize,
                  getChunkStartX(),
                  getChunkStartY());
    bakeStaticNavObstacles();
    mNavStaticsDirty = false;
}

void Level1::bakeStaticNavObstacles()
{
    std::vector<Entity*> statics;
    statics.reserve(mActiveTrees.size() + mRocks.size() + 1);

    for (Tree* tree : mActiveTrees)
    {
        if (tree && tree->getIsActive())
        {
            statics.push_back(tree);
        }
    }

    for (Rock* rock : mRocks)
    {
        if (rock && rock->getIsActive())
        {
            statics.push_back(rock);
        }
    }

    if (mTable && mTable->getIsActive())
    {
        statics.push_back(mTable.get());
    }

    mNavMap.applyStaticObstacles(statics);
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

    std::vector<Dog*> aliveBossMinions;
    for (Dog* dog : mBossMinions)
    {
        if (!dog || dog->isDead())
        {
            if (dog)
            {
                removeCollidableEntity(mCollidableEntities, dog);
                delete dog;
            }
            continue;
        }
        dog->setIsActive(true);
        dog->setCanCollide(true);
        ensureInCollidables(dog);
        mEnemies.push_back(dog);
        aliveBossMinions.push_back(dog);
    }
    mBossMinions.swap(aliveBossMinions);

    if (mBoss && !mBoss->isDead())
    {
        mBoss->setIsActive(true);
        mBoss->setCanCollide(true);
        ensureInCollidables(mBoss);
        mEnemies.push_back(mBoss);
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

    if (isDebugMode())
    {
        mGoldCount += 1000;
        syncGoldSlot();
        LOG_DEBUG("Debug mode: granted 1000 gold for shop testing");
    }
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

    const bool shootersUnlocked = mAttackersUnlocked;
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
            const float typeNoise = mMapGenerator.whiteNoise(worldX, worldY, SHOOTER_SPAWN_SETTINGS.salt + 3u);

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

            const bool spawnShooter = shootersUnlocked && typeNoise >= (1.0f - SHOOTER_SPAWN_SETTINGS.shooterChance);
            int shooterVariant = 0;
            float shooterHeight = dogHeight;
            if (spawnShooter)
            {
                const float heightNoiseShooter = mMapGenerator.whiteNoise(worldX, worldY, SHOOTER_SPAWN_SETTINGS.salt + 4u);
                shooterHeight = SHOOTER_SPAWN_SETTINGS.minHeightPx +
                                heightNoiseShooter * (SHOOTER_SPAWN_SETTINGS.maxHeightPx - SHOOTER_SPAWN_SETTINGS.minHeightPx);
                shooterVariant = std::clamp(GetRandomValue(0, 2), 0, 2);
                AttackEnemy *attacker = new AttackEnemy({worldPosX, worldPosY},
                                                        shooterVariant,
                                                        &mSpreadProjectiles,
                                                        shooterHeight);
                attacker->setNavMap(&mNavMap);
                bucket.push_back(attacker);
                spawnedCount++;
            }
            else
            {
                Dog* dog = new Dog({worldPosX, worldPosY}, variant, dogHeight);
                dog->setNavMap(&mNavMap);
                bucket.push_back(dog);
                spawnedCount++;
            }

            if (isDebugMode())
            {
                const float dist = sqrtf((dx * dx) + (dy * dy));
                const int logVariant = spawnShooter ? shooterVariant : variant;
                const float logHeight = spawnShooter ? shooterHeight : dogHeight;
                LOG_DEBUG(TextFormat("Enemy spawn[%p] chunk=(%d,%d) type=%s variant=%d pos=(%.1f,%.1f) height=%.1f dist=%.1f",
                                     bucket.back(),
                                     chunk.first,
                                     chunk.second,
                                     spawnShooter ? "shooter" : "dog",
                                     logVariant,
                                     worldPosX,
                                     worldPosY,
                                     logHeight,
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
        bakeStaticNavObstacles();
        mNavStaticsDirty = false;
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
