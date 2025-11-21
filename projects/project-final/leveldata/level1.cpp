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
#include "rock.h"

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
        0.995f,
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

    constexpr const char *BRANCH_SLOT_ICON_TAG = "BRANCH";
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
    resetBranchInventory();
    initialiseInventoryUI();
    updateChunkStream(true);
    // Lighting shader temporarily disabled; keep call commented for future restoration.
    // initialiseLightingShader();
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
    updatePlayerAttack(deltaTime);
    updateMeleeTimer(deltaTime);

    updateCameraFromPlayer(deltaTime);
    updateGameOverState();
    updateInventoryUI(deltaTime);
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
    }
    EndMode2D();

    // if (lightingShader)
    // {
    //     lightingShader->end();
    // }

    drawPlayerHUD();
    drawInventoryOverlay();
    drawGameOverOverlay();
    DrawFPS(0, 60);
}

void Level1::shutdown()
{
    LOG_INFO("Level1 shutdown");
    mInventoryBar.reset();
    mInventory.reset();
    clearMusicNotes();
    clearEnemies();
    clearBranches();
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
    if (defeated && isDebugMode())
    {
        LOG_DEBUG(TextFormat("Music attack defeated enemy[%p] damage=%d", target, damage));
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
                enemy->applyDamage(projectile->getDamage());
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
        target->applyDamage(mMeleeDamage);
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

    const char *branchText = TextFormat("Branches: %d/%d", mBranchInventory, mBranchCapacity);
    DrawRectangleRounded({16.0f, 20.0f, 160.0f, 28.0f}, 0.3f, 4, Fade(BLACK, 0.4f));
    DrawText(branchText, 24, 26, 18, DARKGREEN);
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

void Level1::initialiseInventoryUI()
{
    mInventory = std::make_unique<Inventory>(Inventory::DEFAULT_SLOT_COUNT);
    mBranchSlotIndex = 0;

    InventorySlot branchSlot;
    branchSlot.id = "branches";
    branchSlot.label = "Branches";
    branchSlot.iconTint = DARKGREEN;
    branchSlot.quantity = mBranchInventory;

    mInventory->setSlot(mBranchSlotIndex, branchSlot);
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
    tryThrowBranchAtEnemy();
}

void Level1::handleMeleeAttackAction()
{
    if (mIsGameOver)
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
    mInitialBranchCount = std::clamp(branch::PRESET_INITIALS[clamped], 0, branch::MAX_HELD);
    mBoxBranchReward = std::max(1, branch::PRESET_BOX_REWARDS[clamped]);
    resetBranchInventory();
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
