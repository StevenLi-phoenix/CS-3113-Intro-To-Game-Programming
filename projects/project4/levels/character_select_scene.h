#ifndef CHARACTER_SELECT_SCENE_H
#define CHARACTER_SELECT_SCENE_H

#include "../lib/Scene.h"

#include <vector>
#include <utility>
#include <string>

class CharacterSelectScene : public Scene
{
private:
    std::vector<std::pair<std::string, std::string>> mVariants;
    int mCurrentIndex = 0;

public:
    CharacterSelectScene();
    ~CharacterSelectScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // CHARACTER_SELECT_SCENE_H
