#include "raylib.h"
// #include "lib/vector_ops.h"
// #include "lib/pid_controller.h"
#include "lib/helper.h"

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
struct Constants {
    const char *TITLE = "Hello raylib!";
    int SCREEN_WIDTH = 800 * 1.5f;
    int SCREEN_HEIGHT = 450 * 1.5f;
    int FPS = 60;
};

// Global State
struct GameState {
};

// Global Variables
AppStatus gAppStatus   = RUNNING;
GameState g;
Constants c;

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
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {
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