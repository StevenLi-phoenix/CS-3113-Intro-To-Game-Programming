#ifndef LEVEL1_H
#define LEVEL1_H

#include "../lib/Scene.h"
#include "../lib/Helper.h"
#include "../lib/mapgenerator.h"
#include "../lib/Inventory.h"
#include "../lib/ui/InventoryBar.h"
#include "../constants.h"
#include "player.h"
#include "tree.h"
#include "dog.h"
#include "music_note.h"
#include "../lib/Enemy.h"
#include "../lib/NavMap.h"
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

class Branch;
class Box;
class Rock;

// Simple test level scene. Add real game logic later.
class Level1 final : public Scene
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
    void resolveBranchImpacts();
    void cleanupBranches();
    void resetBranchInventory();
    bool consumeBranch();
    void addBranches(int amount);
    void initialiseInventoryUI();
    void updateInventoryUI(float deltaTime);
    void drawInventoryOverlay();
    void syncBranchSlot();
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
    void cleanupInactiveTrees();
    struct ChunkKeyHash
    {
        size_t operator()(const std::pair<int, int> &key) const
        {
            // Spread bits to reduce collisions across adjacent chunks
            return (static_cast<size_t>(key.first) * 73856093u) ^
                   (static_cast<size_t>(key.second) * 19349663u);
        }
    };
    void spawnEnemiesForChunk(const std::pair<int, int> &chunk,
                              std::vector<Enemy*> &bucket,
                              const Vector2 &playerPos);

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
    Vector2 mPlayerSpawnPosition = c::ORIGIN;
    unsigned int mWorldSeed = 1337u;
    MapGenerator mMapGenerator;
    std::vector<unsigned int> mLevelData;
    NavMap mNavMap;
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

    int mBranchInventory = branch::DEFAULT_INITIAL;
    int mInitialBranchCount = branch::DEFAULT_INITIAL;
    int mBranchCapacity = branch::MAX_HELD;
    int mBoxBranchReward = branch::DEFAULT_BOX_REWARD;
    float mMeleeTimer = 0.0f;
    float mMeleeCooldown = combat::MELEE_COOLDOWN;
    float mMeleeRange = combat::MELEE_RANGE;
    float mMeleeDamage = combat::MELEE_DAMAGE;

    std::unique_ptr<Inventory> mInventory;
    std::unique_ptr<InventoryBar> mInventoryBar;
    size_t mBranchSlotIndex = 0;
};

#endif
