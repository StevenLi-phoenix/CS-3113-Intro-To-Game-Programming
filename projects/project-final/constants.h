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
    constexpr float MELEE_RANGE = 80.0f;
    constexpr float MELEE_COOLDOWN = 0.45f;
    constexpr float MELEE_DAMAGE = 1.0f;
}

namespace branch {
    inline constexpr float THROW_SPEED = 520.0f;
    inline constexpr float THROW_RANGE = 520.0f;
    inline constexpr float MIN_THROW_DISTANCE = 24.0f;
    inline constexpr float PROJECTILE_DAMAGE = 1.0f;
    inline constexpr int MAX_HELD = 12;
    inline constexpr int DEFAULT_INITIAL = 5;
    inline constexpr int DEFAULT_BOX_REWARD = 4;
    inline constexpr int DIFFICULTY_PRESET_COUNT = 4;
    inline constexpr int PRESET_INITIALS[DIFFICULTY_PRESET_COUNT] = {8, 5, 3, 2};
    inline constexpr int PRESET_BOX_REWARDS[DIFFICULTY_PRESET_COUNT] = {6, 4, 3, 2};
}

namespace lighting {
    constexpr float DEFAULT_DAY_LENGTH_SECONDS = 180.0f;
    constexpr float MIN_AMBIENT_INTENSITY = 0.25f;
    constexpr float MAX_AMBIENT_INTENSITY = 1.0f;
    constexpr float SHADOW_BASE_STRENGTH = 0.35f;
    constexpr float VIGNETTE_DAY_RADIUS = 0.9f;
    constexpr float VIGNETTE_NIGHT_RADIUS = 0.35f;
    constexpr float VIGNETTE_SOFTNESS = 0.4f;
    constexpr float CURSOR_LIGHT_RADIUS = 0.085f;
    constexpr float CURSOR_LIGHT_INTENSITY = 0.9f;
}

namespace pathfinding {
    constexpr int REQUEST_BUDGET_PER_FRAME = 12;
    constexpr float SOFT_RESERVATION_BASE_COST = 10.0f;
}

namespace physics {
    constexpr float PUSH_RADIUS_SCALE = 1.08f;
    constexpr float PUSH_IMPULSE = 65.0f;
    constexpr float COLLISION_CELL_SIZE = 192.0f;
    constexpr float COLLISION_QUERY_MARGIN = 16.0f;
}

#endif
