#include "raylib.h"
#include "lib/helper.h"
#include "lib/Scene.h"
#include "levels/level_witchdemo.h"
#include "levels/level0.h"


// Global Constants
struct Constants {
    const char *TITLE = "Witch Character Demo";
    constexpr static int SCREEN_WIDTH = 800 * 1.5f;
    constexpr static int SCREEN_HEIGHT = 450 * 1.5f;
    constexpr static int FPS = 60;
};


// Global Variables
AppStatus gAppStatus   = RUNNING;
Constants c;
Scene* gCurrentScene = nullptr;
float gDeltaTime = 0.0f;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(c.SCREEN_WIDTH, c.SCREEN_HEIGHT, c.TITLE);
    SetTargetFPS(c.FPS);

    gCurrentScene = new Level0();
    gCurrentScene->initialise();
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    gDeltaTime = GetFrameTime();

    if (gCurrentScene) {
        gCurrentScene->update(gDeltaTime);
    }
}

void render()
{
    BeginDrawing();

    if (gCurrentScene) {
        gCurrentScene->render();
    }

    EndDrawing();
}

void shutdown()
{
    if (gCurrentScene) {
        delete gCurrentScene;
        gCurrentScene = nullptr;
    }

    CloseWindow(); // Close window and OpenGL context
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
