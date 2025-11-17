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

#include "constants.h"
#include "leveldata/player.h"
#include "lib/Helper.h"



AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;
float gTimeAccumulator = 0.0f;

void initialise();
void processInput();
void update();
void render();
void shutdown();

Player* player;

void initialise()
{
    InitWindow(c::SCREEN_WIDTH, c::SCREEN_HEIGHT, c::TITLE);
    SetTargetFPS(c::FPS);

    player = new Player();
    player->setIsActive(true);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    if (IsKeyDown(KEY_A)) player->moveLeft();
    if (IsKeyDown(KEY_D)) player->moveRight();
    if (IsKeyDown(KEY_W)) player->moveUp();
    if (IsKeyDown(KEY_S)) player->moveDown();
}

void update()
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;
    gTimeAccumulator += deltaTime;
    while (gTimeAccumulator >= c::FIXED_TIMESTEP)
    {
        player->update(c::FIXED_TIMESTEP);
        gTimeAccumulator -= c::FIXED_TIMESTEP;
    }
    LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    player->render();
    player->displayCollider();
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
    delete player;
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
