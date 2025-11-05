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
    const int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, static_cast<int>(mOrigin.x) - titleWidth / 2, static_cast<int>(mOrigin.y) - 180, titleFontSize, RAYWHITE);

    const char *subtitle = "Press Enter to Start";
    const int subtitleFont = 32;
    const int subtitleWidth = MeasureText(subtitle, subtitleFont);
    DrawText(subtitle, static_cast<int>(mOrigin.x) - subtitleWidth / 2, static_cast<int>(mOrigin.y) - 100, subtitleFont, LIGHTGRAY);

    const char *instructions = "Use arrow keys in-game. Esc quits.";
    const int instructionFont = 20;
    const int instructionWidth = MeasureText(instructions, instructionFont);
    DrawText(instructions, static_cast<int>(mOrigin.x) - instructionWidth / 2, static_cast<int>(mOrigin.y) + 40, instructionFont, GRAY);
}

void StartMenuScene::shutdown()
{
    // Nothing to clean up for start menu
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}
