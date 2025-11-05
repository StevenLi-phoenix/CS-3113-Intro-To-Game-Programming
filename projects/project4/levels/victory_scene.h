#ifndef VICTORY_SCENE_H
#define VICTORY_SCENE_H

#include "../lib/Scene.h"

class VictoryScene : public Scene
{
public:
    VictoryScene() = default;
    ~VictoryScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // VICTORY_SCENE_H
