#ifndef LEVEL_THREE_H
#define LEVEL_THREE_H

#include "level_base.h"

class LevelThree : public LevelBase
{
private:
    static constexpr int LEVEL_WIDTH = 44;
    static constexpr int LEVEL_HEIGHT = 14;
    static const unsigned int LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

public:
    LevelThree();
    ~LevelThree() override = default;

protected:
    const unsigned int *getLevelData() const override { return LEVEL_DATA; }
    int getLevelWidth() const override { return LEVEL_WIDTH; }
    int getLevelHeight() const override { return LEVEL_HEIGHT; }
    Vector2 getSpawnTile() const override { return {2.5f, 9.0f}; }
    Rectangle getGoalTileArea() const override { return {33.0f, 9.0f, 3.5f, 2.5f}; }
    void setupEnemies() override;
    void onLevelCompleted() override;
    void renderForeground() override;
};

#endif // LEVEL_THREE_H
