#ifndef LEVEL1_H
#define LEVEL1_H

#include "../lib/Scene.h"
#include "../lib/Helper.h"
#include "../lib/mapgenerator.h"
#include "../constants.h"
#include "player.h"
#include "tree.h"
#include <vector>

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
    void updateChunkStream(bool forceRebuild = false);
    void drawChunkDebug();
    void ensureTileTexture();
    void ensureTreeAtlas();

    Player* mPlayer = nullptr;
    Map* mMap = nullptr;
    std::vector<Tree*> mTrees;
    std::vector<Entity*> mCollidableEntities; // All collidable entities (trees, rocks, etc.)
    unsigned int mWorldSeed = 1337u;
    MapGenerator mMapGenerator;
    std::vector<unsigned int> mLevelData;

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
    Texture2D mTileTexture{};
    bool mTileTextureReady = false;
};

#endif
