#ifndef LEVEL1_H
#define LEVEL1_H

#include "../lib/Scene.h"
#include "../lib/Helper.h"
#include "../lib/mapgenerator.h"
#include "../constants.h"
#include "player.h"
#include "tree.h"
#include "dog.h"
#include "music_note.h"
#include "../lib/Enemy.h"
#include "../lib/NavMap.h"
#include <array>
#include <vector>
#include <unordered_map>
#include <utility>

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

    Player* getPlayer() const { return mPlayer; }

private:
    void buildProceduralMap();
    void generateTrees();
    void generateEnemies();
    void updateChunkStream(bool forceRebuild = false);
    void drawChunkDebug();
    void ensureTileTexture();
    void ensureTreeAtlas();
    void ensureMusicNotes();
    void refreshMusicNoteSlots();
    void updateMusicNoteBehaviour();
    void updatePlayerAttack(float deltaTime);
    void spawnMusicNoteForSlot(size_t slotIndex);
    void spawnPlayer();
    void clearTrees();
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
    std::vector<Tree*> mTrees;
    std::vector<Enemy*> mEnemies;
    std::vector<Entity*> mCollidableEntities; // All collidable entities (trees, rocks, etc.)
    std::vector<MusicNote*> mMusicNotes;
    unsigned int mWorldSeed = 1337u;
    MapGenerator mMapGenerator;
    std::vector<unsigned int> mLevelData;
    NavMap mNavMap;

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
};

#endif
