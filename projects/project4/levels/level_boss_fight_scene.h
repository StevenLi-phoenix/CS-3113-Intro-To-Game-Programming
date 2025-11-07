#ifndef LEVEL_BOSS_FIGHT_SCENE_H
#define LEVEL_BOSS_FIGHT_SCENE_H

#include "level_base.h"

class BossFightScene : public LevelBase
{
private:
    static constexpr int LEVEL_WIDTH = 36;
    static constexpr int LEVEL_HEIGHT = 12;
    static const unsigned int LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];
    static constexpr int BOSS_MAX_HEALTH = 5;
    static constexpr float VICTORY_DELAY = 1.2f;

    Entity *mBoss = nullptr;
    int mBossHealth = BOSS_MAX_HEALTH;
    float mInvulnerabilityTimer = 0.0f;
    bool mBossDefeated = false;
    float mVictoryTimer = 0.0f;
    Vector2 mLastPlayerPosition {0.0f, 0.0f};

protected:
    const unsigned int *getLevelData() const override { return LEVEL_DATA; }
    int getLevelWidth() const override { return LEVEL_WIDTH; }
    int getLevelHeight() const override { return LEVEL_HEIGHT; }
    Vector2 getSpawnTile() const override { return {5.0f, 8.5f}; }
    Rectangle getGoalTileArea() const override { return {0.0f, 0.0f, 0.0f, 0.0f}; }

    void setupEnemies() override;
    void renderHUD() const override;
    bool onPlayerEnemyCollision(Entity &enemy) override;
    void onLevelReset() override;

public:
    BossFightScene();
    ~BossFightScene() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void shutdown() override;
};

#endif // BOSS_FIGHT_SCENE_H
