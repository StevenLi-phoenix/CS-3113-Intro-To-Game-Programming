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
#include "leveldata/Settings.h"
#include "leveldata/level1.h"
#include "lib/Helper.h"
#include "lib/Controller.h"
#include "lib/Music.h"

AppStatus gAppStatus = RUNNING;
float gTimeAccumulator = 0.0f;
int gFixedStepsThisFrame = 0;

void initialise();
void processInput();
void update();
void render();
void shutdown();

Controller* gController = nullptr;
Settings* gSettings = nullptr;
Level1* gLevel1 = nullptr;
bool gShowSettings = false;

void initialise()
{
    InitWindow(c::SCREEN_WIDTH, c::SCREEN_HEIGHT, c::TITLE);
    SetTargetFPS(c::FPS);
    AudioManager::init();

    gLevel1 = new Level1();
    gLevel1->initialise();

    Player* levelPlayer = gLevel1->getPlayer();
    gController = new Controller();
    if (levelPlayer)
    {
        gController->bindAction("move_left", KEY_A, Controller::InputEvent::Held, [levelPlayer](float) {
            levelPlayer->moveLeft();
        });
        gController->bindAction("move_right", KEY_D, Controller::InputEvent::Held, [levelPlayer](float) {
            levelPlayer->moveRight();
        });
        gController->bindAction("move_up", KEY_W, Controller::InputEvent::Held, [levelPlayer](float) {
            levelPlayer->moveUp();
        });
        gController->bindAction("move_down", KEY_S, Controller::InputEvent::Held, [levelPlayer](float) {
            levelPlayer->moveDown();
        });
    }

    gSettings = new Settings(levelPlayer, gController);
    gSettings->initialise();
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    if (IsKeyPressed(KEY_F1) && gSettings)
    {
        gShowSettings = !gShowSettings;
        gSettings->setVisible(gShowSettings);
        if (gController)
        {
            gController->setInputCaptureActive(gShowSettings);
        }
    }
}

void update()
{
    float deltaTime = getDeltaTime();
    gFixedStepsThisFrame = 0;
    gTimeAccumulator += deltaTime;
    if (gController)
    {
        gController->update(deltaTime);
    }
    AudioManager::update();
    while (gTimeAccumulator >= c::FIXED_TIMESTEP)
    {
        if (gController && !gShowSettings)
        {
            gController->update(c::FIXED_TIMESTEP);
        }
        if (gLevel1)
        {
            gLevel1->update(c::FIXED_TIMESTEP);
        }
        gTimeAccumulator -= c::FIXED_TIMESTEP;
        gFixedStepsThisFrame++;
    }

    if (gShowSettings && gSettings)
    {
        gSettings->update(deltaTime);
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
            LOG_DEBUG(TextFormat("Frame: dt=%.3f steps=%d accumulator=%.3f", deltaTime, gFixedStepsThisFrame, gTimeAccumulator));
        }
    }
    // LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (gLevel1)
    {
        gLevel1->render();
    }

    if (gShowSettings && gSettings)
    {
        gSettings->render();
    }
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
    delete gSettings;
    gSettings = nullptr;
    delete gController;
    gController = nullptr;
    if (gLevel1)
    {
        gLevel1->shutdown();
        delete gLevel1;
        gLevel1 = nullptr;
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
