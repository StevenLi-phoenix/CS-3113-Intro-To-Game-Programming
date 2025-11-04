#ifndef LEVEL_EXAMPLE_H
#define LEVEL_EXAMPLE_H

#include "../lib/Scene.h"

// ===========================
// LEVEL DIMENSIONS
// ===========================
constexpr int LEVEL_WIDTH = 14,   // Number of tiles horizontally
              LEVEL_HEIGHT = 8;   // Number of tiles vertically

class LevelExample : public Scene {
private:
    // ===========================
    // LEVEL DATA (TILE MAP)
    // ===========================
    // 1D array representing 2D grid (row-major order)
    // 0 = empty, 1-4 = different tile types
    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,  // Row 0 (top)
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,  // Row 1
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,  // Row 2
        4, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 4,  // Row 3
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 4,  // Row 4
        4, 2, 2, 2, 0, 0, 0, 2, 2, 2, 3, 3, 3, 4,  // Row 5
        4, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3, 3, 3, 4,  // Row 6
        4, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3, 3, 3, 4   // Row 7 (bottom)
    };

public:
    // ===========================
    // CONSTANTS
    // ===========================
    static constexpr float TILE_DIMENSION       = 75.0f;    // Size of each tile in pixels
    static constexpr float ACCELERATION_OF_GRAVITY = 981.0f; // Gravity acceleration (pixels/s²)
    static constexpr float END_GAME_THRESHOLD   = 800.0f;   // Y-position threshold for level end/death

    // ===========================
    // CONSTRUCTORS & DESTRUCTOR
    // ===========================
    LevelExample();                                      // Default constructor
    LevelExample(Vector2 origin, const char *bgHexCode); // Constructor with origin and background color
    ~LevelExample();                                     // Destructor

    // ===========================
    // OVERRIDDEN SCENE METHODS
    // ===========================
    void initialise() override;        // Initialize level resources (map, player, camera, etc.)
    void update(float deltaTime) override; // Update level logic each frame
    void render() override;            // Render level graphics
    void shutdown() override;          // Clean up level resources
};

#endif // LEVEL_EXAMPLE_H
