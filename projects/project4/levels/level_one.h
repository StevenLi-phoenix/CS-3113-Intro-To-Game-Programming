#ifndef LEVEL_ONE_H
#define LEVEL_ONE_H

#include "level_base.h"

class LevelOne : public LevelBase
{
private:
    static constexpr int LEVEL_WIDTH = 36;
    static constexpr int LEVEL_HEIGHT = 12;
    static const unsigned int LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

public:
    LevelOne();
    ~LevelOne() override = default;

protected:
    const unsigned int *getLevelData() const override { return LEVEL_DATA; }
    int getLevelWidth() const override { return LEVEL_WIDTH; }
   int getLevelHeight() const override { return LEVEL_HEIGHT; }
    Vector2 getSpawnTile() const override { return {2.5f, 8.5f}; }
    Rectangle getGoalTileArea() const override { return {32.0f, 8.5f, 3.0f, 2.5f}; }
    void setupEnemies() override;
    void renderForeground() override;
};

#endif // LEVEL_ONE_H
