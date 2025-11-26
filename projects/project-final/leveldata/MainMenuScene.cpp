#include "MainMenuScene.h"
#include "LevelSelectScene.h"
#include "CreditsScene.h"
#include <memory>
#include "../lib/SceneController.h"
#include "../constants.h"
#include "raylib.h"

extern SceneController* gSceneController;
extern AppStatus gAppStatus;

namespace
{
    constexpr float kButtonSpacing = 74.0f;
    constexpr float kButtonWidth = 320.0f;
    constexpr float kButtonHeight = 58.0f;
}

void MainMenuScene::initialise()
{
    resetCamera();
    setCameraFollowEnabled(false);
    mTitlePulseTime = 0.0f;
    buildMenu();
}

void MainMenuScene::buildMenu()
{
    clearButtons();

    Vector2 center = { c::SCREEN_WIDTH / 2.0f, c::SCREEN_HEIGHT / 2.0f };
    const int buttonCount = 4;
    float totalSpan = (buttonCount - 1) * kButtonSpacing;
    float startY = center.y - totalSpan * 0.3f;

    addMenuButton("Play", startY, []() {
        if (gSceneController)
        {
            gSceneController->requestSceneChange(std::make_unique<LevelSelectScene>());
        }
    });

    addMenuButton("Settings", startY + kButtonSpacing, []() {
        if (gSceneController)
        {
            gSceneController->toggleSettings();
        }
    });

    addMenuButton("Credits", startY + kButtonSpacing * 2.0f, []() {
        if (gSceneController)
        {
            gSceneController->requestSceneChange(std::make_unique<CreditsScene>());
        }
    });

    addMenuButton("Quit", startY + kButtonSpacing * 3.0f, []() {
        gAppStatus = TERMINATED;
    });
}

void MainMenuScene::clearButtons()
{
    for (auto &button : mButtons)
    {
        if (button)
        {
            button->shutdown();
        }
    }
    mButtons.clear();
}

void MainMenuScene::addMenuButton(const std::string &label, float y, const Button::Callback &callback)
{
    Vector2 position = { c::SCREEN_WIDTH / 2.0f, y };
    Vector2 size = { kButtonWidth, kButtonHeight };

    auto button = std::make_unique<Button>(position, size, label);
    button->setFontSize(28);
    button->setBackgroundColor(Fade(DARKBLUE, 0.7f));
    button->setBorderColor(WHITE);
    button->setBorderThickness(2.0f);
    button->setTextColor(RAYWHITE);
    button->setOnClick(callback);
    button->setZIndex(1);
    mButtons.emplace_back(std::move(button));
}

void MainMenuScene::update(float deltaTime)
{
    mTitlePulseTime += deltaTime;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        if (gSceneController)
        {
            gSceneController->requestSceneChange(std::make_unique<LevelSelectScene>());
        }
    }

    for (auto &button : mButtons)
    {
        if (button)
        {
            button->update(deltaTime);
        }
    }
}

void MainMenuScene::render()
{
    Color top = { 4, 11, 30, 255 };
    Color bottom = { 1, 4, 10, 255 };
    DrawRectangleGradientV(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, top, bottom);

    const char *title = "Project Final";
    int fontSize = 64;
    float pulse = (sinf(mTitlePulseTime * 1.2f) + 1.0f) * 0.5f;
    Color titleColor = Fade(RAYWHITE, 0.85f + 0.15f * pulse);
    int titleWidth = MeasureText(title, fontSize);
    DrawText(title,
             c::SCREEN_WIDTH / 2 - titleWidth / 2,
             120,
             fontSize,
             titleColor);

    const char *subtitle = "Press F1 for settings • WASD to move • Mouse to interact";
    int subtitleSize = 20;
    int subtitleWidth = MeasureText(subtitle, subtitleSize);
    DrawText(subtitle,
             c::SCREEN_WIDTH / 2 - subtitleWidth / 2,
             190,
             subtitleSize,
             Fade(LIGHTGRAY, 0.9f));

    if (isDebugMode())
    {
        const char *debugLabel = "DEBUG BUILD - instrumentation enabled";
        int debugSize = 18;
        int debugWidth = MeasureText(debugLabel, debugSize);
        DrawText(debugLabel,
                 c::SCREEN_WIDTH / 2 - debugWidth / 2,
                 220,
                 debugSize,
                 Fade(ORANGE, 0.95f));
    }

    for (auto &button : mButtons)
    {
        if (button)
        {
            button->render();
        }
    }
}

void MainMenuScene::shutdown()
{
    clearButtons();
}
