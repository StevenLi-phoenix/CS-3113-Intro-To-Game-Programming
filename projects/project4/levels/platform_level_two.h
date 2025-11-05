#ifndef PLATFORM_LEVEL_TWO_H
#define PLATFORM_LEVEL_TWO_H

#include "platform_level_base.h"

class PlatformLevelTwo : public PlatformLevelBase
{
private:
    static constexpr int LEVEL_WIDTH = 40;
    static constexpr int LEVEL_HEIGHT = 13;
    static const unsigned int LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

public:
    PlatformLevelTwo();
    ~PlatformLevelTwo() override = default;

protected:
    const unsigned int *getLevelData() const override { return LEVEL_DATA; }
    int getLevelWidth() const override { return LEVEL_WIDTH; }
    int getLevelHeight() const override { return LEVEL_HEIGHT; }
    Vector2 getSpawnTile() const override { return {1.5f, 8.5f}; }
    Rectangle getGoalTileArea() const override { return {29.5f, 9.0f, 3.5f, 2.5f}; }
    void renderForeground() override;
};

#endif // PLATFORM_LEVEL_TWO_H
