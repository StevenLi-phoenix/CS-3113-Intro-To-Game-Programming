#ifndef LEVEL1_H
#define LEVEL1_H

#include "../lib/Scene.h"
#include "../lib/Helper.h"
#include "../lib/mapgenerator.h"
#include "../lib/Inventory.h"
#include "../lib/ui/InventoryBar.h"
#include "../lib/ui/Button.h"
#include "../constants.h"
#include "player.h"
#include "DifficultyConfig.h"
#include "ResourceTags.h"
#include "tree.h"
#include "dog.h"
#include "spread_projectile.h"
#include "compass.h"
#include "table_with_map.h"
#include "music_note.h"
#include "../lib/Enemy.h"
#include "../lib/NavMap.h"
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

class Branch;
class Box;
class Rock;
class GoldCoin;

// Simple test level scene. Add real game logic later.
class Level1 : public Scene
{
public:
    Level1() = default;
    ~Level1() = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
    Player* getPlayer() override { return mPlayer; }
    void handleRetryAction() override;
    void onRetryBindingChanged(KeyboardKey key) override;
    void handlePrimaryAttackAction() override;
    void onDifficultyPresetChanged(int index) override;
    void handleMeleeAttackAction() override;

    Player* getPlayer() const { return mPlayer; }

private:
    void buildProceduralMap();
    void generateRocks();
    void generateEnemies();
    void updateChunkStream(bool forceRebuild = false);
    void bakeStaticNavObstacles();
    void refreshNavMeshStatics();
    void drawChunkDebug();
    void ensureTileTexture();
    void ensureTreeAtlas();
    void ensureMusicNotes();
    void drawPlayerHUD() const;
    void refreshMusicNoteSlots();
    void updateMusicNoteBehaviour();
    void updatePlayerAttack(float deltaTime);
    void updateMeleeTimer(float deltaTime);
    void spawnMusicNoteForSlot(size_t slotIndex);
    void spawnPlayer();
    void clearTrees();
    void clearRocks();
    void clearEnemies();
    void clearMusicNotes();
    void updateCameraFromPlayer(float deltaTime);
    MapGenerator::GenerationSettings buildGeneratorSettings() const;
    Vector2 computeMapOrigin() const;
    void rebuildMap(const Vector2 &origin);
    Enemy* findNearestEnemy(float maxRange) const;
    MusicNote* findAvailableNoteForVariant(MusicNote::Variant variant);
    bool hasEnemyWithinRadius(float radius) const;
    MusicNote::Variant currentNoteVariant() const;
    void advanceNoteVariant();
    void updateGameOverState();
    void drawGameOverOverlay() const;
    void resetPlayerForRetry();
    void updatePlayerSpawnPoint(const Vector2 &position);
    void handleMouseBranchInput();
    bool tryThrowBranchAt(const Vector2 &worldTarget);
    bool tryThrowBranchAtEnemy();
    void tryMeleeAttack();
    Enemy* findNearestMeleeTarget(float range) const;
    void applyMeleeDamage(Enemy *target);
    void spawnGoldCoin(const Vector2 &position);
    void updateGoldCoins();
    void collectGoldCoin(GoldCoin *coin);
    void handleEnemyDefeated(Enemy *enemy);
    void resolveBranchImpacts();
    void cleanupBranches();
    void resetBranchInventory();
    bool consumeBranch();
    void addBranches(int amount);
    void addPotions(int amount);
    void applyDifficulty(const DifficultyState &state);
    bool usePotion();
    void initialiseInventoryUI();
    void updateInventoryUI(float deltaTime);
    void drawInventoryOverlay();
    void syncBranchSlot();
    void syncGoldSlot();
    void syncWeaponSlot();
    void syncPotionSlot();
    void updateTreesForStream();
    void updateBoxesForStream();
    void spawnBoxesForChunk(const std::pair<int, int> &chunk,
                            std::vector<Box*> &bucket);
    void spawnTreesForChunk(const std::pair<int, int> &chunk,
                            std::vector<Tree*> &bucket);
    void updateBoxRewards();
    void collectBox(Box *box);
    void clearBoxes();
    void clearBranches();
    void clearGoldCoins(bool resetCount = true);
    void cleanupInactiveTrees();
    void updateTutorialOverlay(float deltaTime);
    void drawTutorialOverlay() const;
    bool tutorialInputDetected() const;
    float tutorialOverlayAlpha() const;
    void spawnQuestTarget();
    int requiredGold() const;
    void updateQuestState();
    void drawQuestLog() const;
    void drawCompassIndicator();
    void drawMapTableUI() const;
    void updateShop(float deltaTime);
    void drawShopOverlay() const;
    bool isPlayerNearTable(float radius) const;
    void spawnBoss();
    void spawnBossMinion();
    void updateBossFight(float deltaTime);
    void drawBossBar() const;
    void drawBossDirectionIndicator() const;
    void drawBossSummonEffects();
    void updateBossSummonEffects(float deltaTime);
    void updateHurtOverlay(float deltaTime);
    void drawHurtOverlay();
    void updateMeleeEffects(float deltaTime);
    void drawMeleeEffects();
    void spawnMeleeEffect();
    void updateSpreadProjectiles(float deltaTime);
    void drawSpreadProjectiles() const;
    void ensureHurtShader();
    void cleanupBossMinions();
    void pruneEnemyList(Enemy *enemy);
    void ensureShopUI();
    void updateShopButtonsLayout();
    void handleShopClose();
    void updateBranchPickups();
    bool isBranchSelected() const;
    bool isCompassSelected() const;
    bool isAxeSelected() const;
    bool isPotionSelected() const;
    void handleBossDefeated();
    virtual std::unique_ptr<Scene> createNextSceneAfterBoss();
    void updatePostBossShooters(float deltaTime);
    void spawnShooterEnemy();
    struct ChunkKeyHash
    {
        size_t operator()(const std::pair<int, int> &key) const
        {
            // Spread bits to reduce collisions across adjacent chunks
            return (static_cast<size_t>(key.first) * 73856093u) ^
                   (static_cast<size_t>(key.second) * 19349663u);
        }
    };
    virtual void spawnEnemiesForChunk(const std::pair<int, int> &chunk,
                                      std::vector<Enemy*> &bucket,
                                      const Vector2 &playerPos);

protected:
    virtual float enemyGoldDropChance() const;
    const MapGenerator& getMapGenerator() const { return mMapGenerator; }
    float getTileSize() const { return mTileSize; }
    const NavMap* getNavMap() const { return &mNavMap; }
    std::vector<SpreadProjectile>& getSpreadProjectiles() { return mSpreadProjectiles; }

private:
    std::unordered_map<std::pair<int, int>, std::vector<Enemy*>, ChunkKeyHash> mChunkEnemies;
    Player* mPlayer = nullptr;
    Map* mMap = nullptr;
    std::unordered_map<std::pair<int, int>, std::vector<Tree*>, ChunkKeyHash> mChunkTrees;
    std::vector<Tree*> mActiveTrees;
    std::vector<Rock*> mRocks;
    std::vector<Enemy*> mEnemies;
    std::vector<Entity*> mCollidableEntities; // All collidable entities (trees, rocks, etc.)
    std::vector<MusicNote*> mMusicNotes;
    std::vector<Branch*> mBranches;
    std::vector<Box*> mBoxes;
    std::vector<GoldCoin*> mGoldCoins;
    std::unique_ptr<TableWithMap> mTable;
    std::unique_ptr<Compass> mCompassUI;
    Vector2 mPlayerSpawnPosition = c::ORIGIN;
    unsigned int mWorldSeed = 1337u;
    MapGenerator mMapGenerator;
    std::vector<unsigned int> mLevelData;
    NavMap mNavMap;
    bool mNavStaticsDirty = false;
    std::unordered_map<std::pair<int, int>, std::vector<Box*>, ChunkKeyHash> mChunkBoxes;

    const int mChunkSize = 64;
    const int mChunkLoadRadius = 1; // 1 chunk in every direction (3x3)
    int mMapColumns = mChunkSize * 3;
    int mMapRows = mChunkSize * 3;
    const float mTileSize = 32.0f;
    const int mTileColumns = 16;
    const int mTileRows = 1;
    const unsigned int mNoiseSalt = 0x8da6b343u; // decorrelate hash noise
    const char *mMapTexturePath = "assets/ElderAsset1.2.png";
    const char *mMapAtlasMetadataPath = "assets/atlas_refined.json";
    Rectangle mTileAtlasRegion = {0.0f, 432.0f, 512.0f, 32.0f};
    Texture2D *mTileTexture = nullptr;
    bool mTileTextureReady = false;

    float mAttackTimer = 0.0f;
    float mAttackIntervalSeconds = combat::PLAYER_ATTACK_INTERVAL;
    float mAttackRange = combat::PLAYER_ATTACK_RANGE;
    float mNoteIdleRadius = combat::NOTE_IDLE_DETECTION_RADIUS;
    int mNoteCount = combat::NOTE_DEFAULT_COUNT;
    std::array<MusicNote::Variant, 4> mNoteSequence = {
        MusicNote::Variant::Note1,
        MusicNote::Variant::Note2,
        MusicNote::Variant::Note3,
        MusicNote::Variant::Star
    };
    size_t mNoteSequenceIndex = 0;
    bool mIsGameOver = false;
    KeyboardKey mRetryBindingKey = KEY_ENTER;
    bool mSkipPlayerChunkForNextEnemySpawn = false;

    int mBranchInventory = branch::DEFAULT_INITIAL;
    DifficultyState mDifficulty;
    int mInitialBranchCount = branch::DEFAULT_INITIAL;
    int mBranchCapacity = branch::MAX_HELD;
    int mBoxBranchReward = branch::DEFAULT_BOX_REWARD;
    float mMeleeTimer = 0.0f;
    float mMeleeCooldown = combat::MELEE_COOLDOWN;
    float mMeleeRange = combat::MELEE_RANGE;
    float mMeleeDamage = combat::MELEE_DAMAGE;
    int mPotionCount = 0;
    int mPotionCapacity = 3;
    float mBranchDamage = branch::PROJECTILE_DAMAGE;
    int mSwordUpgradeCount = 0;
    int mShurikenUpgradeCount = 0;
    bool mRecoverableThrows = false;
    bool mShopOpen = false;
    bool mShopSuppressed = false;

    std::unique_ptr<Inventory> mInventory;
    std::unique_ptr<InventoryBar> mInventoryBar;
    size_t mAxeSlotIndex = 0;
    size_t mCompassSlotIndex = 1;
    size_t mBranchSlotIndex = 2;
    size_t mGoldSlotIndex = 3;
    size_t mPotionSlotIndex = 4;
    int mGoldCount = 0;

    std::string mQuestDescription = "Survive and find the table with map";
    bool mQuestComplete = false;
    bool mBossSpawned = false;
    bool mBossDefeated = false;
    bool mBossAdvanceRequested = false;
    bool mShooterPhaseActive = false;
    int mShootersRemaining = 0;
    float mShooterSpawnTimer = 0.0f;
    Enemy *mBoss = nullptr;
    std::vector<Dog*> mBossMinions;
    float mBossSummonTimer = 0.0f;
    std::vector<std::unique_ptr<Button>> mShopButtons;
    float mBossRepathTimer = 0.0f;
    struct SummonEffect
    {
        Vector2 origin{0.0f, 0.0f};
        float elapsed = 0.0f;
        float duration = 1.0f;
        float startRadius = 24.0f;
        float endRadius = 140.0f;
    };
    std::vector<SummonEffect> mBossSummonEffects;

    struct MeleeEffect
    {
        Vector2 position{0.0f, 0.0f};
        float elapsed = 0.0f;
        float duration = 0.35f;
        int frameCount = 4;
        bool facesLeft = false;
        float scale = 1.0f;
    };
    std::vector<MeleeEffect> mMeleeEffects;

    std::vector<SpreadProjectile> mSpreadProjectiles;

    float mHurtOverlayTimer = 0.0f;
    float mLastPlayerHealth = PlayerConstants::MAX_HEALTH;
    ShaderProgram mHurtShader;
    bool mHurtShaderReady = false;

    bool mTutorialOverlayVisible = false;
    bool mTutorialOverlayDismissed = false;
    float mTutorialOverlayDisplayTimer = 0.0f;
    float mTutorialOverlayFadeTimer = 0.0f;
    float mTutorialReopenHintTimer = 0.0f;
};

#endif
