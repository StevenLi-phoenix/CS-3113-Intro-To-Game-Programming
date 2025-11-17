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
}

#endif

