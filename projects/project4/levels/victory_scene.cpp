#include "victory_scene.h"

#include "../lib/game_context.h"
#include "../lib/helper.h"

void VictoryScene::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#1E2F23";
    mGameState.nextSceneID = 0;
}

void VictoryScene::update(float /*deltaTime*/)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::START_MENU);
    }
}

void VictoryScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "You Win!";
    const int titleFont = 64;
    const int titleWidth = MeasureText(title, titleFont);
    DrawText(title, static_cast<int>(mOrigin.x) - titleWidth / 2, static_cast<int>(mOrigin.y) - 130, titleFont, LIME);

    const char *message = "You cleared every stage.";
    const int messageFont = 28;
    const int messageWidth = MeasureText(message, messageFont);
    DrawText(message, static_cast<int>(mOrigin.x) - messageWidth / 2, static_cast<int>(mOrigin.y) - 40, messageFont, RAYWHITE);

    const char *prompt = "Press Enter to return to the main menu";
    const int promptFont = 24;
    const int promptWidth = MeasureText(prompt, promptFont);
    DrawText(prompt, static_cast<int>(mOrigin.x) - promptWidth / 2, static_cast<int>(mOrigin.y) + 40, promptFont, LIGHTGRAY);
}

void VictoryScene::shutdown()
{
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}
