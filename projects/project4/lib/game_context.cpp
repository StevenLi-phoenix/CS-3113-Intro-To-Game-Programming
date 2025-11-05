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
    gContext.maxLives = 3;
    gContext.lives = gContext.maxLives;
    gContext.currentLevelIndex = 0;
    gContext.selectedVariant = "Fire";
    gContext.paused = false;
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
}

bool HasPendingSceneChange()
{
    return gContext.pendingScene != gContext.currentScene;
}

SceneID ConsumePendingSceneChange()
{
    gContext.currentScene = gContext.pendingScene;
    return gContext.currentScene;
}
