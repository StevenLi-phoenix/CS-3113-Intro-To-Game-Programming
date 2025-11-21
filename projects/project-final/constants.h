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

namespace combat {
    constexpr float PLAYER_ATTACK_INTERVAL = 0.9f;   // seconds between auto-attacks
    constexpr float PLAYER_ATTACK_RANGE    = 240.0f; // max distance to hit enemies
    constexpr float NOTE_FOLLOW_LAG        = 0.08f;  // seconds worth of velocity to lag behind
    constexpr float NOTE_FOLLOW_LERP       = 9.5f;   // smoothing multiplier
    constexpr float NOTE_BOB_AMPLITUDE     = 6.0f;
    constexpr float NOTE_BOB_SPEED         = 2.3f;
    constexpr float NOTE_DESIRED_HEIGHT    = 36.0f;
    constexpr float NOTE_ORBIT_RADIUS      = 34.0f;
    constexpr float NOTE_ORBIT_SPEED       = 1.7f;   // radians per second
    constexpr float NOTE_IDLE_DETECTION_RADIUS = 260.0f;
    constexpr float NOTE_ATTACK_TRAVEL_TIME    = 0.18f;
    constexpr float NOTE_ATTACK_RETURN_TIME    = 0.14f;
    constexpr int NOTE_DEFAULT_COUNT = 0;
}

#endif

