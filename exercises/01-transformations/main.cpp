#include "raylib.h"
#include <math.h>

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH        = 800 * 1.5f,
              SCREEN_HEIGHT       = 450 * 1.5f,
              FPS                 = 60,
              SIDES               = 3;

constexpr float RADIUS          = 200.0f, // radius of the orbit
                ORBIT_SPEED     = 0.05f,  // the speed at which the triangle will travel its orbit
                BASE_SIZE       = 50,     // the size of the triangle when it's not being scaled
                MAX_AMPLITUDE   = 1.5f,  // by how much the triangle will be expanding/contracting
                PULSE_SPEED     = 100.0f, // how fast the triangle is going to be "pulsing"
                PULSE_INCREMENT = 1.0f;  // the current value we're scaling by

constexpr Vector2 ORIGIN = { 
    SCREEN_WIDTH  / 2, 
    SCREEN_HEIGHT / 2
};

// new
enum PulseDirection {SMALLER = -1, BIGGER = 1};
PulseDirection gpulseDirection = BIGGER;

// Global Variables
AppStatus gAppStatus = RUNNING;

float gScaleFactor   = BASE_SIZE,
      gAngle         = 0.0f,
      gPulseTime     = 0.0f;
Vector2 gPosition    = ORIGIN;

float gOrbitLocation = 0.0f;

// Function Declarations
Color ColorFromHex(const char *hex);
void initialise();
void processInput();
void update();
void render();
void shutdown();

#include <stdio.h>
#include <stdlib.h>


// You can ignore this function, it's just to get nice colours :)
Color ColorFromHex(const char *hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    // Default alpha = 255 (opaque)
    unsigned int r = 0, 
                 g = 0, 
                 b = 0, 
                 a = 255;

    // 6‑digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // 8‑digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}


void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Funny Heartbeat");

    SetTargetFPS(FPS);

    gAngle = 20.0f;
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    /**
     * HEARTBEAT EFFECT
     */
    gScaleFactor += PULSE_INCREMENT * gpulseDirection;
    if (gScaleFactor > MAX_AMPLITUDE * BASE_SIZE) gpulseDirection = SMALLER;
    else if (gScaleFactor < BASE_SIZE) gpulseDirection = BIGGER;

    /**
     * ORBIT EFFECT
     */
    gPulseTime += ORBIT_SPEED;
    gPosition = (Vector2){ 
        RADIUS * cos(gPulseTime) + ORIGIN.x, 
        RADIUS * sin(gPulseTime) + ORIGIN.y 
    };
}

void render()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawPoly(
        gPosition,
        SIDES, 
        gScaleFactor, 
        gAngle, 
        ColorFromHex("#F88379")
    );

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}