#ifndef LEVEL_BASE_H
#define LEVEL_BASE_H

#include <vector>

#include "../lib/Scene.h"
#include "../lib/game_context.h"
#include "witch.h"

class LevelBase : public Scene
{
protected:
    struct EnemyRecord
    {
        Entity *entity = nullptr;
        Vector2 spawnPosition {0.0f, 0.0f};
    };

    static constexpr float TILE_PIXEL_SIZE = 64.0f;
    static constexpr int TILESET_COLUMNS = 16;
    static constexpr int TILESET_ROWS = 16;
    static constexpr const char *TILESET_PATH = "assets/world_tileset.png";

    Map *mMap = nullptr;
    Witch *mWitch = nullptr;
    Vector2 mSpawnPoint {0.0f, 0.0f};
    Rectangle mGoalTileArea {0.0f, 0.0f, 1.0f, 1.0f};
    Rectangle mGoalWorldArea {0.0f, 0.0f, 0.0f, 0.0f};
    std::string mLevelName;
    std::vector<EnemyRecord> mEnemies;
    Sound mHitSound {};
    Sound mDeathSound {};
    Sound mJumpSound {};
    bool mHitSoundLoaded = false;
    bool mDeathSoundLoaded = false;
    bool mJumpSoundLoaded = false;
    bool mResetPending = false;
    bool mGameOverPending = false;
    float mGameOverTimer = 0.0f;
    static constexpr float GAME_OVER_DELAY = 0.9f;

    SceneID mSceneID;
    SceneID mNextSceneID;

    float mCameraZoom = 1.6f;

protected:
    LevelBase(SceneID sceneID, SceneID nextSceneID, const char *levelName);
    ~LevelBase() override;

    virtual const unsigned int *getLevelData() const = 0;
    virtual int getLevelWidth() const = 0;
    virtual int getLevelHeight() const = 0;
    virtual Vector2 getSpawnTile() const = 0;
    virtual Rectangle getGoalTileArea() const = 0;

    void handleInput();
    void updateCamera();
    void updateGoalArea();
    void checkFallBoundary();
    void respawnPlayer();
    bool hasReachedGoal() const;
    Vector2 tileToWorld(const Vector2 &tile) const;
    Vector2 tileCenter(int tileX, int tileY) const;
    void updateEnemies(float deltaTime);
    void renderEnemies();
    bool handlePlayerEnemyCollisions();
    void clearEnemies();
    void registerEnemy(Entity *enemy);
    static void configureSlimeSprite(Entity *slime);
    void resetEnemyPosition(EnemyRecord &record);
    void resetLevelState();
    void loadHitSound();
    void unloadHitSound();
    void loadDeathSound();
    void unloadDeathSound();
    void loadJumpSound();
    void unloadJumpSound();
    virtual bool onPlayerEnemyCollision(Entity &enemy);
    virtual void onLevelReset();

    virtual void onLevelCompleted();
    virtual void renderForeground();
    virtual void renderHUD() const;
    virtual void setupEnemies();

public:
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // LEVEL_BASE_H
