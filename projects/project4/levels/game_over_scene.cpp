#include "game_over_scene.h"

#include "../lib/game_context.h"
#include "../lib/helper.h"

void GameOverScene::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#2C1B28";
    mGameState.nextSceneID = 0;
}

void GameOverScene::update(float /*deltaTime*/)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::START_MENU);
    }
}

void GameOverScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "You Lose!";
    const int titleFont = 64;
    const int titleWidth = MeasureText(title, titleFont);
    DrawText(title, static_cast<int>(mOrigin.x) - titleWidth / 2, static_cast<int>(mOrigin.y) - 120, titleFont, RED);

    const char *message = "You ran out of lives.";
    const int messageFont = 28;
    const int messageWidth = MeasureText(message, messageFont);
    DrawText(message, static_cast<int>(mOrigin.x) - messageWidth / 2, static_cast<int>(mOrigin.y) - 40, messageFont, RAYWHITE);

    const char *prompt = "Press Enter to return to the main menu";
    const int promptFont = 24;
    const int promptWidth = MeasureText(prompt, promptFont);
    DrawText(prompt, static_cast<int>(mOrigin.x) - promptWidth / 2, static_cast<int>(mOrigin.y) + 40, promptFont, LIGHTGRAY);
}

void GameOverScene::shutdown()
{
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}
