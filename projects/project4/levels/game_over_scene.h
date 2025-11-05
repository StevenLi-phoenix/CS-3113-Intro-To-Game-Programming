#ifndef GAME_OVER_SCENE_H
#define GAME_OVER_SCENE_H

#include "../lib/Scene.h"

class GameOverScene : public Scene
{
public:
    GameOverScene() = default;
    ~GameOverScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // GAME_OVER_SCENE_H
