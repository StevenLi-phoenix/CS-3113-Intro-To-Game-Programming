#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <string>

enum class SceneID
{
    START_MENU,
    CHARACTER_SELECT,
    LEVEL_ONE,
    LEVEL_TWO,
    LEVEL_THREE,
    BOSS_FIGHT,
    GAME_OVER,
    VICTORY,
    QUIT
};

struct GameContext
{
    SceneID currentScene = SceneID::START_MENU;
    SceneID pendingScene = SceneID::START_MENU;

    int maxLives = 3;
    int lives = 3;
    int currentLevelIndex = 0;

    std::string selectedVariant = "Fire";

    bool paused = false;
    bool reloadScene = false;
    bool audioReady = false;
};

GameContext &GetGameContext();
void ResetGameContext();
void InitialiseGameContext();
void RequestSceneChange(SceneID scene);
bool HasPendingSceneChange();
SceneID ConsumePendingSceneChange();

#endif // GAME_CONTEXT_H
