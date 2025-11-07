#include "game_context.h"

namespace
{
    GameContext gContext;
}

GameContext &GetGameContext()
{
    return gContext;
}

void ResetGameContext()
{
    const bool audioReady = gContext.audioReady;
    gContext.maxLives = 3;
    gContext.lives = gContext.maxLives;
    gContext.currentLevelIndex = 0;
    gContext.selectedVariant = "Fire";
    gContext.paused = false;
    gContext.reloadScene = false;
    gContext.audioReady = audioReady;
}

void InitialiseGameContext()
{
    ResetGameContext();
    gContext.currentScene = SceneID::START_MENU;
    gContext.pendingScene = SceneID::START_MENU;
}

void RequestSceneChange(SceneID scene)
{
    gContext.pendingScene = scene;
    gContext.reloadScene = (scene == gContext.currentScene);
}

bool HasPendingSceneChange()
{
    return gContext.reloadScene || gContext.pendingScene != gContext.currentScene;
}

SceneID ConsumePendingSceneChange()
{
    gContext.currentScene = gContext.pendingScene;
    gContext.reloadScene = false;
    return gContext.currentScene;
}
