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
#include "lib/ui/Button.h"
#include "lib/ui/ToggleButton.h"
#include <vector>

AppStatus gAppStatus = RUNNING;
float gTimeAccumulator = 0.0f;

void initialise();
void processInput();
void update();
void render();
void shutdown();

Player* player;
Button* testButton;
ToggleButton* testToggle;
std::vector<Entity*> globalUpdateQueue = {};

void initialise()
{
    InitWindow(c::SCREEN_WIDTH, c::SCREEN_HEIGHT, c::TITLE);
    SetTargetFPS(c::FPS);

    player = new Player();
    player->setIsActive(true);
    
    // Create a test button with callback example
    testButton = new Button(
        {c::SCREEN_WIDTH / 2.0f, 100.0f},  // position
        {200.0f, 50.0f},                    // size
        "Click Me!"                         // text
    );
    
    // Set button colors
    testButton->setBackgroundColor(BLUE);
    testButton->setTextColor(WHITE);
    testButton->setBorderColor(DARKBLUE);
    testButton->setBorderThickness(3.0f);
    
    // Set callback function (using lambda)
    testButton->setOnClick([]() {
        LOG("Button clicked! This is a callback example.");
        // You can add any code here that should execute when button is clicked
    });

    // Create a toggle button demo beneath the regular button
    testToggle = new ToggleButton(
        {c::SCREEN_WIDTH / 2.0f, 200.0f},
        {220.0f, 50.0f},
        "Sound ON",
        "Sound OFF"
    );
    testToggle->setOnBackgroundColor(Fade(BLUE, 0.2f));
    testToggle->setOffBackgroundColor(RAYWHITE);
    testToggle->setOnBorderColor(BLUE);
    testToggle->setOffBorderColor(LIGHTGRAY);
    testToggle->setOnTextColor(BLUE);
    testToggle->setOffTextColor(DARKGRAY);
    testToggle->setOnToggle([](bool toggled) {
        if (toggled)
        {
            LOG("ToggleButton switched ON");
        }
        else
        {
            LOG("ToggleButton switched OFF");
        }
    });
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
    float deltaTime = getDeltaTime();
    gTimeAccumulator += deltaTime;
    while (gTimeAccumulator >= c::FIXED_TIMESTEP)
    {
        player->update(c::FIXED_TIMESTEP);
        // Update button to check for clicks
        testButton->update(c::FIXED_TIMESTEP);
        testToggle->update(c::FIXED_TIMESTEP);
        gTimeAccumulator -= c::FIXED_TIMESTEP;
    }
    // LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    player->render();
    player->displayCollider();
    
    // Update global cursor (should be called after all button updates)
    Button::updateGlobalCursor();
    ToggleButton::updateGlobalCursor();
    
    // Render button
    testButton->render();
    testToggle->render();
    
    // Optional: Show mouse over status
    if (testButton->isMouseOver())
    {
        DrawText("Mouse over button!", 10, 10, 20, BLACK);
    }
    
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
    delete player;
    delete testButton;
    delete testToggle;
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
