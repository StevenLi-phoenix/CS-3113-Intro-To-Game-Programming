#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "lib/Helper.h"

namespace c {
    constexpr const char *TITLE = "Project Final";
    constexpr static int SCREEN_WIDTH = 800 * 1.5f;
    constexpr static int SCREEN_HEIGHT = 450 * 1.5f;
    constexpr static int FPS = 60;
    constexpr static Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    constexpr static float FIXED_TIMESTEP = 1.0f / 60.0f;

    // Tree sprite rectangles from atlas_refined.json
    constexpr static int TREE_VARIANT_COUNT = 3;
    constexpr static Rectangle TREE_SPRITES[TREE_VARIANT_COUNT] = {
        { 259.0f, 278.0f, 24.0f, 62.0f },  // TREE variant 0
        { 292.0f, 279.0f, 25.0f, 62.0f },  // TREE variant 1
        { 320.0f, 272.0f, 32.0f, 80.0f }   // TREE variant 2
    };
}

#endif

