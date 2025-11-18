#ifndef LEVEL1_H
#define LEVEL1_H

#include "../lib/Scene.h"
#include "../lib/Helper.h"
#include "../lib/mapgenerator.h"
#include "../constants.h"
#include "player.h"
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

    Player* getPlayer() const { return mPlayer; }

private:
    void buildProceduralMap();

    Player* mPlayer = nullptr;
    Map* mMap = nullptr;
    MapGenerator mMapGenerator;
    std::vector<unsigned int> mLevelData;

    const int mMapColumns = 40;
    const int mMapRows = 24;
    const float mTileSize = 32.0f;
    const int mTileColumns = 16;
    const int mTileRows = 1;
    const char *mMapTexturePath = "assets/ElderAsset1.2.png";
    Rectangle mTileAtlasRegion = {0.0f, 432.0f, 512.0f, 32.0f};
};

#endif
