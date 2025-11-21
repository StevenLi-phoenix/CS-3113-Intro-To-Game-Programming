#ifndef LEVEL_SELECT_SCENE_H
#define LEVEL_SELECT_SCENE_H

#include "../lib/Scene.h"
#include "../lib/ui/ui.h"
#include <memory>
#include <string>
#include <vector>

class LevelSelectScene final : public Scene
{
public:
    LevelSelectScene() = default;
    ~LevelSelectScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    struct LevelButton
    {
        std::unique_ptr<Button> widget;
        std::string description;
    };

    void buildButtons();
    void addLevelButton(const std::string &label,
                        const std::string &description,
                        float y,
                        const Button::Callback &callback);
    void addBackButton(float y);
    void clearButtons();
    void returnToMenu();

    std::vector<LevelButton> mLevelButtons;
    std::unique_ptr<Button> mBackButton;
};

#endif

