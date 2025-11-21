#include "CreditsScene.h"
#include "MainMenuScene.h"
#include <memory>
#include "../lib/SceneController.h"
#include "../constants.h"

extern SceneController* gSceneController;

namespace
{
    constexpr float kLineSpacing = 40.0f;
}

CreditsScene::CreditsScene()
    : mScrollOffset(0.0f),
      mScrollSpeed(40.0f)
{
    mLines = {
        "",
        "Rolling Credits",
        "",
        "Project Final",
        "A CS-3113 Production",
        "",
        "Programming",
        "Steven Li",
        "",
        "Art & Design",
        "Procedural Forest Team",
        "",
        "Audio",
        "Raylib Soundtrack",
        "",
        "Special Thanks",
        "NYU Game Center Faculty",
        "Friends and Playtesters",
        "",
        "",
        "Built with Raylib + LibTorch",
        "Course: Intro to Game Programming",
        "",
        "Press ESC to skip",
        ""
    };
}

void CreditsScene::initialise()
{
    resetCamera();
    setCameraFollowEnabled(false);
    resetScroll();
}

void CreditsScene::resetScroll()
{
    mScrollOffset = static_cast<float>(c::SCREEN_HEIGHT) + 60.0f;
}

float CreditsScene::totalScrollHeight() const
{
    return static_cast<float>(mLines.size()) * kLineSpacing;
}

void CreditsScene::returnToMenu()
{
    if (gSceneController)
    {
        gSceneController->requestSceneChange(std::make_unique<MainMenuScene>());
    }
}

void CreditsScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        returnToMenu();
        return;
    }

    mScrollOffset -= mScrollSpeed * deltaTime;
    if (mScrollOffset < -totalScrollHeight())
    {
        returnToMenu();
    }
}

void CreditsScene::render()
{
    Color top = { 12, 8, 26, 255 };
    Color bottom = { 1, 1, 6, 255 };
    DrawRectangleGradientV(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, top, bottom);

    float y = mScrollOffset;
    for (const auto &line : mLines)
    {
        int fontSize = (line == "Rolling Credits" || line == "Project Final") ? 36 : 22;
        Color color = Fade(RAYWHITE, line.empty() ? 0.0f : 0.9f);
        if (!line.empty())
        {
            int width = MeasureText(line.c_str(), fontSize);
            DrawText(line.c_str(),
                     c::SCREEN_WIDTH / 2 - width / 2,
                     static_cast<int>(y),
                     fontSize,
                     color);
        }
        y += kLineSpacing;
    }

    const char *footer = "Thank you for playing!";
    int footerSize = 20;
    int footerWidth = MeasureText(footer, footerSize);
    DrawText(footer,
             c::SCREEN_WIDTH / 2 - footerWidth / 2,
             c::SCREEN_HEIGHT - 60,
             footerSize,
             Fade(LIGHTGRAY, 0.85f));
}

void CreditsScene::shutdown()
{
    // nothing to release yet
}

