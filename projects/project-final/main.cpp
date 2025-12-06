/**
* Author: Steven Li
* Assignment: In the woods
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
#include "leveldata/MainMenuScene.h"
#include "lib/Helper.h"
#include "lib/SceneController.h"
#include "lib/Music.h"
#include <memory>

AppStatus gAppStatus = RUNNING;
float gTimeAccumulator = 0.0f;
int gFixedStepsThisFrame = 0;

void initialise();
void processInput();
void update();
void render();
void shutdown();

SceneController* gSceneController = nullptr;

void initialise()
{
    InitWindow(c::SCREEN_WIDTH, c::SCREEN_HEIGHT, c::TITLE);
    SetTargetFPS(c::FPS);
    AudioManager::init();

    gSceneController = new SceneController();
    gSceneController->initialise(std::make_unique<MainMenuScene>());
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    if (IsKeyPressed(KEY_F1) && gSceneController)
    {
        gSceneController->toggleSettings();
    }
}

void update()
{
    float deltaTime = getDeltaTime();
    gFixedStepsThisFrame = 0;
    gTimeAccumulator += deltaTime;
    if (gSceneController)
    {
        gSceneController->updateInput(deltaTime);
    }
    AudioManager::update();
    while (gTimeAccumulator >= c::FIXED_TIMESTEP)
    {
        if (gSceneController)
        {
            gSceneController->updateFixed(c::FIXED_TIMESTEP);
        }
        gTimeAccumulator -= c::FIXED_TIMESTEP;
        gFixedStepsThisFrame++;
    }

    if (gSceneController)
    {
        gSceneController->updateFrame(deltaTime);
    }

    if (isDebugMode())
    {
        const bool bigDt = deltaTime > 0.033f; // ~30fps
        const bool manySteps = gFixedStepsThisFrame > 1;
        if (bigDt || manySteps)
        {
            LOG_INFO(TextFormat("Frame spike: dt=%.3f steps=%d accumulator=%.3f", deltaTime, gFixedStepsThisFrame, gTimeAccumulator));
        }
        else
        {
            // TODO: remove this
            // LOG_DEBUG(TextFormat("Frame: dt=%.3f steps=%d accumulator=%.3f", deltaTime, gFixedStepsThisFrame, gTimeAccumulator));
        }
    }
    // LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (gSceneController)
    {
        gSceneController->render();
    }
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
    if (gSceneController)
    {
        gSceneController->shutdown();
        delete gSceneController;
        gSceneController = nullptr;
    }
    AudioManager::shutdown();
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
