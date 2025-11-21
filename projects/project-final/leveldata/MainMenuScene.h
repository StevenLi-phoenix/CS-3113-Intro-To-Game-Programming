#ifndef MAIN_MENU_SCENE_H
#define MAIN_MENU_SCENE_H

#include "../lib/Scene.h"
#include "../lib/ui/ui.h"
#include <memory>
#include <vector>
#include <string>

class MainMenuScene final : public Scene
{
public:
    MainMenuScene() = default;
    ~MainMenuScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    void buildMenu();
    void clearButtons();
    void addMenuButton(const std::string &label, float y, const Button::Callback &callback);

    std::vector<std::unique_ptr<Button>> mButtons;
    float mTitlePulseTime = 0.0f;
};

#endif

