#include "character_select_scene.h"

#include "../lib/game_context.h"
#include "../lib/helper.h"

CharacterSelectScene::CharacterSelectScene() : Scene()
{
}

void CharacterSelectScene::initialise()
{
    mOrigin = {
        GetScreenWidth()  / 2.0f,
        GetScreenHeight() / 2.0f
    };

    mBGColourHexCode = "#1B2A41";
    mGameState.nextSceneID = 0;

    mVariants = {
        {"Fire", "Fire"},
        {"Grass", "Grass"},
        {"Light", "Light"},
        {"Water", "Water"},
        {"normal", "Arcane"},
        {"green skin", "Forest Spirit"}
    };

    mCurrentIndex = 0;
}

void CharacterSelectScene::update(float /*deltaTime*/)
{
    const int total = static_cast<int>(mVariants.size());
    if (total == 0)
    {
        RequestSceneChange(SceneID::LEVEL_ONE);
        return;
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    {
        mCurrentIndex = (mCurrentIndex - 1 + total) % total;
    }

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        mCurrentIndex = (mCurrentIndex + 1) % total;
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE))
    {
        RequestSceneChange(SceneID::START_MENU);
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        GameContext &ctx = GetGameContext();
        ctx.selectedVariant = mVariants[mCurrentIndex].first;
        ctx.lives = ctx.maxLives;
        ctx.currentLevelIndex = 0;
        ctx.paused = false;
        RequestSceneChange(SceneID::LEVEL_ONE);
    }
}

void CharacterSelectScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "Select Your Witch";
    const int titleFont = 48;
    const int titleWidth = MeasureText(title, titleFont);
    DrawText(title, static_cast<int>(mOrigin.x) - titleWidth / 2, static_cast<int>(mOrigin.y) - 180, titleFont, RAYWHITE);

    if (!mVariants.empty())
    {
        const std::string &displayName = mVariants[mCurrentIndex].second;
        const int fontSize = 42;
        const int textWidth = MeasureText(displayName.c_str(), fontSize);
        DrawText(displayName.c_str(), static_cast<int>(mOrigin.x) - textWidth / 2, static_cast<int>(mOrigin.y) - 60, fontSize, SKYBLUE);
    }

    DrawText("<- / -> to switch", static_cast<int>(mOrigin.x) - 150, static_cast<int>(mOrigin.y) + 20, 24, LIGHTGRAY);
    DrawText("Enter to confirm", static_cast<int>(mOrigin.x) - 130, static_cast<int>(mOrigin.y) + 60, 24, LIGHTGRAY);
    DrawText("Esc to return", static_cast<int>(mOrigin.x) - 110, static_cast<int>(mOrigin.y) + 100, 24, LIGHTGRAY);
}

void CharacterSelectScene::shutdown()
{
    mVariants.clear();
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
}

