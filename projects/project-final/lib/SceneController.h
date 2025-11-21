#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <memory>
#include "Helper.h"
#include "Scene.h"
#include "../leveldata/DifficultyConfig.h"

class Controller;
class Settings;
class Player;

// Manages the active gameplay scene, settings overlay, and future scene switches.
class SceneController
{
public:
    SceneController();
    ~SceneController();

    void initialise(std::unique_ptr<Scene> initialScene);
    void requestSceneChange(std::unique_ptr<Scene> nextScene);
    void applyPendingScene();

    // Per-frame input pass (matches previous behavior of updating controller every frame).
    void updateInput(float deltaTime);
    // Fixed-step gameplay update.
    void updateFixed(float deltaTime);
    // Variable step overlay/update for UI layers.
    void updateFrame(float deltaTime);
    void render();
    void shutdown();

    void toggleSettings();
    bool isSettingsVisible() const { return mSettingsVisible; }

    Controller* getController() { return mController.get(); }
    Scene* getActiveScene() { return mActiveScene.get(); }

private:
    std::unique_ptr<Controller> mController;
    std::unique_ptr<Settings> mSettings;
    std::unique_ptr<Scene> mActiveScene;
    std::unique_ptr<Scene> mPendingScene;
    bool mSettingsVisible = false;
    DifficultyState mDifficultyState{};

    void activateScene(std::unique_ptr<Scene> scene);
    void rebuildSettings(Player* player);
};

#endif
