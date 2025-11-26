#ifndef LEVEL1_CONSTS_H
#define LEVEL1_CONSTS_H

#include "../lib/Helper.h"

namespace level1_consts
{
    constexpr float SHOP_INTERACT_RADIUS = 120.0f;
    constexpr int SWORD_COST = 8;
    constexpr int SHURIKEN_COST = 6;
    constexpr int POTION_COST = 5;
    constexpr float POTION_HEAL_AMOUNT = 4.0f;
    constexpr int POTION_CAPACITY_DEFAULT = 3;
    constexpr float SWORD_DAMAGE_BONUS = 0.2f;
    constexpr float SHURIKEN_DAMAGE_BONUS = 0.2f;
    constexpr float HURT_FLASH_DURATION = 0.65f;
    constexpr float HURT_FLASH_ALPHA = 0.55f;
    constexpr float HURT_FLASH_PULSE = 16.0f;
    constexpr float MELEE_FX_DURATION = 0.32f;
    constexpr float MELEE_FX_FORWARD_OFFSET = 34.0f;
    constexpr float SPREAD_BALL_SPEED = 260.0f;
    constexpr float SPREAD_BALL_DAMAGE = 1.0f;
    constexpr float SPREAD_BALL_RADIUS = 12.0f;
    constexpr float SPREAD_BALL_LIFETIME = 3.0f;
    constexpr int SPREAD_BALL_FRAMES = 6;
    constexpr float SPREAD_BALL_FRAME_TIME = 0.08f;

    constexpr KeyboardKey TUTORIAL_REOPEN_KEY = KEY_F2;
    constexpr float TUTORIAL_REOPEN_HINT_SECONDS = 6.0f;
}

namespace tutorial
{
    constexpr float AUTO_HIDE_SECONDS = 10.0f;
    constexpr float FADE_SECONDS = 1.0f;
    constexpr int MAX_GAMEPADS = 4;
    constexpr const char *TITLE = "Getting Started";
    constexpr const char *LINES[] = {
        "WASD or Arrow Keys to move your character",
        "Left click anywhere to toss a branch at that spot",
        "Press Z to use your equipped weapon (throw or melee)",
        "Press F1 for settings, rebinding, and tips",
        "Seek the table with a map to reach the next level"
    };
    constexpr size_t LINE_COUNT = sizeof(LINES) / sizeof(LINES[0]);
}

#endif // LEVEL1_CONSTS_H
