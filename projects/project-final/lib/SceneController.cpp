#include "SceneController.h"
#include "Controller.h"
#include "../leveldata/Settings.h"
#include "../leveldata/player.h"

SceneController::SceneController() = default;

SceneController::~SceneController()
{
    shutdown();
}

void SceneController::initialise(std::unique_ptr<Scene> initialScene)
{
    mController = std::make_unique<Controller>();
    activateScene(std::move(initialScene));
}

void SceneController::requestSceneChange(std::unique_ptr<Scene> nextScene)
{
    mPendingScene = std::move(nextScene);
}

void SceneController::applyPendingScene()
{
    if (mPendingScene)
    {
        activateScene(std::move(mPendingScene));
        mPendingScene.reset();
    }
}

void SceneController::updateInput(float deltaTime)
{
    applyPendingScene();
    if (mController)
    {
        mController->update(deltaTime);
    }
}

void SceneController::updateFixed(float deltaTime)
{
    applyPendingScene();
    if (!mSettingsVisible && mController)
    {
        mController->update(deltaTime);
    }
    if (mActiveScene)
    {
        mActiveScene->update(deltaTime);
    }
}

void SceneController::updateFrame(float deltaTime)
{
    applyPendingScene();
    if (mSettingsVisible && mSettings)
    {
        mSettings->update(deltaTime);
    }
}

void SceneController::render()
{
    if (mActiveScene)
    {
        mActiveScene->render();
    }
    if (mSettingsVisible && mSettings)
    {
        mSettings->render();
    }
}

void SceneController::shutdown()
{
    if (mSettings)
    {
        mSettings->shutdown();
        mSettings.reset();
    }
    if (mActiveScene)
    {
        mActiveScene->shutdown();
        mActiveScene.reset();
    }
    mController.reset();
    mPendingScene.reset();
    mSettingsVisible = false;
}

void SceneController::toggleSettings()
{
    mSettingsVisible = !mSettingsVisible;
    if (mSettings)
    {
        mSettings->setVisible(mSettingsVisible);
    }
    if (mController)
    {
        mController->setInputCaptureActive(mSettingsVisible);
    }
}

void SceneController::activateScene(std::unique_ptr<Scene> scene)
{
    if (mActiveScene)
    {
        mActiveScene->shutdown();
    }
    mActiveScene = std::move(scene);

    mSettingsVisible = false;
    if (mController)
    {
        mController->setInputCaptureActive(false);
    }
    if (mSettings)
    {
        mSettings->setVisible(false);
    }

    if (mActiveScene)
    {
        mActiveScene->initialise();
    }

    Player* player = mActiveScene ? mActiveScene->getPlayer() : nullptr;
    bindPlayerActions(player);
    rebuildSettings(player);
}

void SceneController::bindPlayerActions(Player* player)
{
    if (!mController) return;

    mController->unbindAction("move_left");
    mController->unbindAction("move_right");
    mController->unbindAction("move_up");
    mController->unbindAction("move_down");

    if (!player) return;

    mController->bindAction("move_left", KEY_A, Controller::InputEvent::Held, [player](float) {
        player->moveLeft();
    });
    mController->bindAction("move_right", KEY_D, Controller::InputEvent::Held, [player](float) {
        player->moveRight();
    });
    mController->bindAction("move_up", KEY_W, Controller::InputEvent::Held, [player](float) {
        player->moveUp();
    });
    mController->bindAction("move_down", KEY_S, Controller::InputEvent::Held, [player](float) {
        player->moveDown();
    });
}

void SceneController::rebuildSettings(Player* player)
{
    if (mSettings)
    {
        mSettings->shutdown();
        mSettings.reset();
    }
    if (!player || !mController)
    {
        return;
    }

    mSettings = std::make_unique<Settings>(player, mController.get());
    mSettings->initialise();
    mSettingsVisible = false;
    mSettings->setVisible(false);
}
