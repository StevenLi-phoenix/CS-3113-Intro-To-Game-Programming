/**
* Author: Steven Li
* Assignment: Project Final
* Date due: 2025-12-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef DEBUG_BUILD
#define DEBUG_BUILD 0
#endif

#include "lib/Helper.h"
#include "lib/Entity.h"
#include "lib/Map.h"
#include "lib/pid_controller.h"
#include "lib/Scene.h"
#include "lib/ShaderProgram.h"
#include "lib/Effects.h"

namespace {
    struct Constants {
        const char *TITLE = "Project Final";
        constexpr static int SCREEN_WIDTH = 800 * 1.5f;
        constexpr static int SCREEN_HEIGHT = 450 * 1.5f;
        constexpr static int FPS = 60;
    };
}

AppStatus gAppStatus = RUNNING;
Constants c;

void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(c.SCREEN_WIDTH, c.SCREEN_HEIGHT, c.TITLE);
    SetTargetFPS(c.FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
}
int main(int argc, char *argv[])
{
    init_log_level(argc, argv);
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
