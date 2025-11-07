#include "start_menu_scene.h"

#include "../lib/game_context.h"
#include "../lib/helper.h"

StartMenuScene::StartMenuScene() : Scene()
{
}

void StartMenuScene::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#202533";
    mGameState.nextSceneID = 0;
}

void StartMenuScene::update(float /*deltaTime*/)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::CHARACTER_SELECT);
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        RequestSceneChange(SceneID::QUIT);
    }
}

void StartMenuScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "Witch Platformer";
    const int titleFontSize = 64;
    DrawText(title, static_cast<int>(mOrigin.x) - 220, static_cast<int>(mOrigin.y) - 180, titleFontSize, RAYWHITE);

    const char *subtitle = "Press Enter to Start";
    const int subtitleFont = 32;
    DrawText(subtitle, static_cast<int>(mOrigin.x) - 160, static_cast<int>(mOrigin.y) - 100, subtitleFont, LIGHTGRAY);

    const char *instructions = "Use arrow keys in-game. Esc quits.";
    const int instructionFont = 20;
    DrawText(instructions, static_cast<int>(mOrigin.x) - 190, static_cast<int>(mOrigin.y) + 40, instructionFont, GRAY);
}

void StartMenuScene::shutdown()
{
    // Nothing to clean up for start menu
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}
