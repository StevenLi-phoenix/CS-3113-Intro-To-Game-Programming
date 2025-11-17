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
#include "lib/ui/TextInput.h"
#include "lib/ui/Dropdown.h"
#include "lib/ui/ListBox.h"
#include <vector>

AppStatus gAppStatus = RUNNING;
float gTimeAccumulator = 0.0f;

void initialise();
void processInput();
void update();
void render();
void shutdown();

Player* player;
TextInput* testTextInput = nullptr;
Dropdown* testDropdown = nullptr;
ListBox* testListBox = nullptr;
std::vector<Entity*> globalUpdateQueue = {};

void initialise()
{
    InitWindow(c::SCREEN_WIDTH, c::SCREEN_HEIGHT, c::TITLE);
    SetTargetFPS(c::FPS);

    player = new Player();
    player->setIsActive(true);

    testTextInput = new TextInput(
        {c::SCREEN_WIDTH / 2.0f, 420.0f},
        {320.0f, 40.0f}
    );
    testTextInput->setPlaceholder("Type something and press Enter");
    testTextInput->setOnSubmit([](const std::string& text) {
        LOG(TextFormat("Submitted text: %s", text.c_str()));
    });

    testDropdown = new Dropdown(
        {c::SCREEN_WIDTH / 2.0f, 260.0f},
        {240.0f, 40.0f}
    );
    testDropdown->setOptions({"Easy", "Medium", "Hard"});
    testDropdown->setOnSelectionChanged([](int index, const std::string& value) {
        LOG(TextFormat("Dropdown selected %d -> %s", index, value.c_str()));
    });

    testListBox = new ListBox(
        {c::SCREEN_WIDTH / 4.0f, 360.0f},
        {220.0f, 160.0f}
    );
    testListBox->setItems({"Red Potion", "Blue Potion", "Green Potion", "Elixir"});
    testListBox->setOnSelectionChanged([](int index, const std::string& value) {
        LOG(TextFormat("ListBox selected %d -> %s", index, value.c_str()));
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
        gTimeAccumulator -= c::FIXED_TIMESTEP;
    }
    if (testTextInput)
    {
        testTextInput->update(deltaTime);
    }
    if (testDropdown)
    {
        testDropdown->update(deltaTime);
    }
    if (testListBox)
    {
        testListBox->update(deltaTime);
    }
    // LOG("Hello from LOG()");
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    player->render();
    player->displayCollider();
    if (testDropdown)
    {
        testDropdown->render();
        DrawText(
            TextFormat("Dropdown choice: %s",
                (testDropdown->getSelectedIndex() >= 0
                    ? testDropdown->getOptions()[testDropdown->getSelectedIndex()].c_str()
                    : "None")),
            20,
            20,
            20,
            DARKGRAY
        );
    }
    if (testListBox)
    {
        testListBox->render();
        DrawText(
            TextFormat("ListBox selection: %s",
                (testListBox->getSelectedIndex() >= 0
                    ? testListBox->getItems()[testListBox->getSelectedIndex()].c_str()
                    : "None")),
            20,
            50,
            20,
            DARKGRAY
        );
    }
    if (testTextInput)
    {
        testTextInput->render();
        DrawText(
            TextFormat("Current input: %s", testTextInput->getText().c_str()),
            20,
            80,
            20,
            DARKGRAY
        );
    }
    
    EndDrawing();
}

void shutdown()
{
    CloseWindow();
    delete player;
    delete testTextInput;
    delete testDropdown;
    delete testListBox;
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
