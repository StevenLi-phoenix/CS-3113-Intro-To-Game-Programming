#ifndef LEVEL_WITCHDEMO_H
#define LEVEL_WITCHDEMO_H

#include "../lib/Scene.h"
#include "witch.h"

class LevelWitchDemo : public Scene
{
private:
    Witch *mWitch = nullptr;

    void handleInput();
    void renderOverlay() const;

public:
    LevelWitchDemo();
    ~LevelWitchDemo() override;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
