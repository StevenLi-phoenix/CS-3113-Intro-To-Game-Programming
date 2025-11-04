#ifndef LEVEL0_H
#define LEVEL0_H

#include "../lib/Scene.h"
#include "witch.h"

// Dimensions for the sample level grid
constexpr int LEVEL0_WIDTH  = 16;
constexpr int LEVEL0_HEIGHT = 10;

class Level0 : public Scene
{
private:
    static constexpr float TILE_PIXEL_SIZE = 64.0f;

    Map   *mMap   = nullptr;
    Witch *mWitch = nullptr;

    unsigned int mLevelData[LEVEL0_WIDTH * LEVEL0_HEIGHT] = {
        // Row 0 (top)
         4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
        // Row 1
         4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
        // Row 2
         4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
        // Row 3
         4,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  4,
        // Row 4
         4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
        // Row 5
         4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
        // Row 6
         4,  0,  0,  3,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
        // Row 7
         4,  0,  0,  3,  0,  3,  0,  0,  0,  0,  2,  2,  0,  0,  0,  4,
        // Row 8
         4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,
        // Row 9 (bottom)
         4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
    };

    void handleInput();
    void updateCamera();

public:
    Level0();
    ~Level0() override;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // LEVEL0_H
