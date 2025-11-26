#include "LevelSelectScene.h"
#include "level1.h"
#include "level2.h"
#include "MainMenuScene.h"
#include <memory>
#include "../lib/SceneController.h"
#include "../constants.h"

extern SceneController* gSceneController;

namespace
{
    constexpr float kButtonWidth = 360.0f;
    constexpr float kButtonHeight = 54.0f;
    constexpr float kButtonSpacing = 80.0f;
}

void LevelSelectScene::initialise()
{
    resetCamera();
    setCameraFollowEnabled(false);
    buildButtons();
}

void LevelSelectScene::buildButtons()
{
    clearButtons();
    float startY = c::SCREEN_HEIGHT / 2.0f - kButtonSpacing;

    addLevelButton("Level 1: Forest Grove",
                   "Battle through the overgrown grove and test combat basics.",
                   startY,
                   []() {
                       if (gSceneController)
                       {
                           gSceneController->requestSceneChange(std::make_unique<Level1>());
                       }
                   });

    addLevelButton("Level 2: Spreadshot Frontier",
                   "ATTACK1-3 shooters wield spreadballs and drop more gold.",
                   startY + kButtonSpacing,
                   []() {
                       if (gSceneController)
                       {
                           gSceneController->requestSceneChange(std::make_unique<Level2>());
                       }
                   });

    addBackButton(startY + kButtonSpacing * 2.0f);
}

void LevelSelectScene::addLevelButton(const std::string &label,
                                      const std::string &description,
                                      float y,
                                      const Button::Callback &callback)
{
    Vector2 position = { c::SCREEN_WIDTH / 2.0f, y };
    Vector2 size = { kButtonWidth, kButtonHeight };
    LevelButton entry;
    entry.widget = std::make_unique<Button>(position, size, label);
    entry.widget->setFontSize(24);
    entry.widget->setBackgroundColor(Fade(DARKGREEN, 0.7f));
    entry.widget->setBorderColor(GREEN);
    entry.widget->setBorderThickness(2.0f);
    entry.widget->setTextColor(RAYWHITE);
    entry.widget->setOnClick(callback);
    entry.description = description;
    mLevelButtons.emplace_back(std::move(entry));
}

void LevelSelectScene::addBackButton(float y)
{
    Vector2 position = { c::SCREEN_WIDTH / 2.0f, y };
    Vector2 size = { 220.0f, 48.0f };
    mBackButton = std::make_unique<Button>(position, size, "Back to Main Menu");
    mBackButton->setFontSize(20);
    mBackButton->setBackgroundColor(Fade(DARKBLUE, 0.7f));
    mBackButton->setBorderColor(BLUE);
    mBackButton->setBorderThickness(2.0f);
    mBackButton->setTextColor(RAYWHITE);
    mBackButton->setOnClick([this]() { returnToMenu(); });
}

void LevelSelectScene::clearButtons()
{
    for (auto &entry : mLevelButtons)
    {
        if (entry.widget)
        {
            entry.widget->shutdown();
        }
    }
    mLevelButtons.clear();

    if (mBackButton)
    {
        mBackButton->shutdown();
        mBackButton.reset();
    }
}

void LevelSelectScene::returnToMenu()
{
    if (gSceneController)
    {
        gSceneController->requestSceneChange(std::make_unique<MainMenuScene>());
    }
}

void LevelSelectScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        returnToMenu();
    }

    for (auto &entry : mLevelButtons)
    {
        if (entry.widget)
        {
            entry.widget->update(deltaTime);
        }
    }

    if (mBackButton)
    {
        mBackButton->update(deltaTime);
    }
}

void LevelSelectScene::render()
{
    Color top = { 6, 20, 12, 255 };
    Color bottom = { 2, 8, 4, 255 };
    DrawRectangleGradientV(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, top, bottom);

    const char *title = "Select a Level";
    int fontSize = 48;
    int titleWidth = MeasureText(title, fontSize);
    DrawText(title,
             c::SCREEN_WIDTH / 2 - titleWidth / 2,
             110,
             fontSize,
             Fade(RAYWHITE, 0.95f));

    const char *hint = "Click a level to jump in. Press ESC to go back.";
    int hintSize = 20;
    int hintWidth = MeasureText(hint, hintSize);
    DrawText(hint,
             c::SCREEN_WIDTH / 2 - hintWidth / 2,
             170,
             hintSize,
             Fade(LIGHTGRAY, 0.9f));

    for (const auto &entry : mLevelButtons)
    {
        if (!entry.widget) continue;

        entry.widget->render();

        const std::string &text = entry.description;
        if (!text.empty())
        {
            Vector2 pos = entry.widget->getPosition();
            int descSize = 18;
            int descWidth = MeasureText(text.c_str(), descSize);
            DrawText(text.c_str(),
                     pos.x - descWidth / 2,
                     pos.y + static_cast<int>(kButtonHeight / 2) + 12,
                     descSize,
                     Fade(LIGHTGRAY, 0.9f));
        }
    }

    if (mBackButton)
    {
        mBackButton->render();
    }
}

void LevelSelectScene::shutdown()
{
    clearButtons();
}
