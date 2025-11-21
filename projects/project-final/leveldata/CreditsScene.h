#ifndef CREDITS_SCENE_H
#define CREDITS_SCENE_H

#include "../lib/Scene.h"
#include <string>
#include <vector>

class CreditsScene final : public Scene
{
public:
    CreditsScene();
    ~CreditsScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    void resetScroll();
    void returnToMenu();
    float totalScrollHeight() const;

    std::vector<std::string> mLines;
    float mScrollOffset;
    float mScrollSpeed;
};

#endif

