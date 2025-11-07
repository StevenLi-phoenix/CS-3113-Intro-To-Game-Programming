#include "raylib.h"
#include "lib/helper.h"
#include "lib/Scene.h"
#include "lib/game_context.h"
#include "levels/start_menu_scene.h"
#include "levels/character_select_scene.h"
#include "levels/level_one.h"
#include "levels/level_two.h"
#include "levels/level_three.h"
#include "levels/level_boss_fight_scene.h"
#include "levels/game_over_scene.h"
#include "levels/victory_scene.h"

#include <array>

#ifndef DEBUG_BUILD
#define DEBUG_BUILD 0
#endif

struct Constants {
    const char *TITLE = "Witch Character Demo";
    constexpr static int SCREEN_WIDTH = 800 * 1.5f;
    constexpr static int SCREEN_HEIGHT = 450 * 1.5f;
    constexpr static int FPS = 60;
};

AppStatus gAppStatus   = RUNNING;
Constants c;
Scene* gCurrentScene = nullptr;
float gDeltaTime = 0.0f;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
float gPreviousTicks = 0.0f;
float gTimeAccumulator = 0.0f;
Music gBackgroundMusic{};
bool gAudioInitialised = false;
bool gBackgroundMusicReady = false;

namespace
{
    constexpr size_t SceneIndex(SceneID id)
    {
        return static_cast<size_t>(id);
    }

    constexpr size_t SCENE_CACHE_SIZE = SceneIndex(SceneID::QUIT) + 1;
    std::array<Scene*, SCENE_CACHE_SIZE> gSceneCache{};
}

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void changeScene(SceneID scene);
void buildSceneCache();
Scene* getCachedScene(SceneID scene);
void handleDebugSceneShortcuts();

// Function Definitions
void initialise()
{
    InitWindow(c.SCREEN_WIDTH, c.SCREEN_HEIGHT, c.TITLE);
    SetTargetFPS(c.FPS);
    InitAudioDevice();
    gAudioInitialised = true;

    if (gAudioInitialised)
    {
        gBackgroundMusic = LoadMusicStream("assets/sound/04_theimperialfleet.wav");
        if (gBackgroundMusic.stream.buffer)
        {
            gBackgroundMusic.looping = true;
            PlayMusicStream(gBackgroundMusic);
            gBackgroundMusicReady = true;
        }
        else
        {
            TraceLog(LOG_WARNING, "Failed to load background music: assets/sound/04_theimperialfleet.wav");
        }
    }
    else
    {
        TraceLog(LOG_WARNING, "Audio device failed to initialise.");
    }

    InitialiseGameContext();
    GetGameContext().audioReady = gAudioInitialised;
    gPreviousTicks = static_cast<float>(GetTime());
    gTimeAccumulator = 0.0f;
    buildSceneCache();
    changeScene(SceneID::START_MENU);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
    handleDebugSceneShortcuts();
}

void update()
{
    const float currentTicks = static_cast<float>(GetTime());
    float deltaTime = currentTicks - gPreviousTicks;
    gPreviousTicks = currentTicks;

    deltaTime += gTimeAccumulator;
    if (gBackgroundMusicReady)
    {
        UpdateMusicStream(gBackgroundMusic);
    }

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        if (gCurrentScene) {
            gCurrentScene->update(FIXED_TIMESTEP);
        }
        deltaTime -= FIXED_TIMESTEP;
        gDeltaTime = FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;

    if (HasPendingSceneChange()) {
        SceneID nextScene = ConsumePendingSceneChange();
        if (nextScene == SceneID::QUIT) {
            gAppStatus = TERMINATED;
        } else {
            changeScene(nextScene);
        }
    }
}

void render()
{
    BeginDrawing();

    if (gCurrentScene) {
        gCurrentScene->render();
    }

    EndDrawing();
}

void shutdown()
{
    if (gCurrentScene) {
        gCurrentScene->shutdown();
        gCurrentScene = nullptr;
    }

    for (Scene* scene : gSceneCache) {
        if (!scene) continue;
        scene->shutdown();
        delete scene;
    }
    gSceneCache.fill(nullptr);

    if (gBackgroundMusicReady)
    {
        StopMusicStream(gBackgroundMusic);
        UnloadMusicStream(gBackgroundMusic);
        gBackgroundMusicReady = false;
    }

    if (gAudioInitialised)
    {
        CloseAudioDevice();
        gAudioInitialised = false;
        GetGameContext().audioReady = false;
    }

    CloseWindow(); // Close window and OpenGL context
}

void changeScene(SceneID scene)
{
    if (scene == SceneID::QUIT) {
        gAppStatus = TERMINATED;
        return;
    }

    if (gCurrentScene) {
        gCurrentScene->shutdown();
        gCurrentScene = nullptr;
    }

    GetGameContext().paused = false;

    gCurrentScene = getCachedScene(scene);
    if (!gCurrentScene) {
        gAppStatus = TERMINATED;
        return;
    }

    gCurrentScene->initialise();
    gTimeAccumulator = 0.0f;
    gPreviousTicks = static_cast<float>(GetTime());
}

void buildSceneCache()
{
    gSceneCache.fill(nullptr);

    gSceneCache[SceneIndex(SceneID::START_MENU)] = new StartMenuScene();
    gSceneCache[SceneIndex(SceneID::CHARACTER_SELECT)] = new CharacterSelectScene();
    gSceneCache[SceneIndex(SceneID::LEVEL_ONE)] = new LevelOne();
    gSceneCache[SceneIndex(SceneID::LEVEL_TWO)] = new LevelTwo();
    gSceneCache[SceneIndex(SceneID::LEVEL_THREE)] = new LevelThree();
    gSceneCache[SceneIndex(SceneID::BOSS_FIGHT)] = new BossFightScene();
    gSceneCache[SceneIndex(SceneID::GAME_OVER)] = new GameOverScene();
    gSceneCache[SceneIndex(SceneID::VICTORY)] = new VictoryScene();
}

Scene* getCachedScene(SceneID scene)
{
    const size_t index = SceneIndex(scene);
    if (index >= gSceneCache.size()) {
        return nullptr;
    }
    return gSceneCache[index];
}

void handleDebugSceneShortcuts()
{
#if DEBUG_BUILD
    bool shortcutTriggered = false;
    if (IsKeyPressed(KEY_ONE))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::START_MENU);
        shortcutTriggered = true;
    }
    else if (IsKeyPressed(KEY_TWO))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::CHARACTER_SELECT);
        shortcutTriggered = true;
    }
    else if (IsKeyPressed(KEY_THREE))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::LEVEL_ONE);
        shortcutTriggered = true;
    }
    else if (IsKeyPressed(KEY_FOUR))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::LEVEL_TWO);
        shortcutTriggered = true;
    }
    else if (IsKeyPressed(KEY_FIVE))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::LEVEL_THREE);
        shortcutTriggered = true;
    }
    else if (IsKeyPressed(KEY_SIX))
    {
        ResetGameContext();
        RequestSceneChange(SceneID::BOSS_FIGHT);
        shortcutTriggered = true;
    }

    if (shortcutTriggered)
    {
        GameContext &ctx = GetGameContext();
        ctx.paused = false;
    }
#endif
}

int main(int argc, char *argv[])
{
    init_log_level(argc, argv);
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
