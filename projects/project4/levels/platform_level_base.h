#ifndef PLATFORM_LEVEL_BASE_H
#define PLATFORM_LEVEL_BASE_H

#include "../lib/Scene.h"
#include "../lib/game_context.h"
#include "witch.h"

class PlatformLevelBase : public Scene
{
protected:
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

    SceneID mSceneID;
    SceneID mNextSceneID;

    float mCameraZoom = 1.6f;

protected:
    PlatformLevelBase(SceneID sceneID, SceneID nextSceneID, const char *levelName);
    ~PlatformLevelBase() override;

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

    virtual void onLevelCompleted();
    virtual void renderForeground();
    virtual void renderHUD() const;

public:
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // PLATFORM_LEVEL_BASE_H
