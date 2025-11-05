#ifndef START_MENU_SCENE_H
#define START_MENU_SCENE_H

#include "../lib/Scene.h"

class StartMenuScene : public Scene
{
public:
    StartMenuScene();
    ~StartMenuScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // START_MENU_SCENE_H
