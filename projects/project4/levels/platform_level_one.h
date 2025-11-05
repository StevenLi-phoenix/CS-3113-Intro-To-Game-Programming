#ifndef PLATFORM_LEVEL_ONE_H
#define PLATFORM_LEVEL_ONE_H

#include "platform_level_base.h"

class PlatformLevelOne : public PlatformLevelBase
{
private:
    static constexpr int LEVEL_WIDTH = 36;
    static constexpr int LEVEL_HEIGHT = 12;
    static const unsigned int LEVEL_DATA[LEVEL_WIDTH * LEVEL_HEIGHT];

public:
    PlatformLevelOne();
    ~PlatformLevelOne() override = default;

protected:
    const unsigned int *getLevelData() const override { return LEVEL_DATA; }
    int getLevelWidth() const override { return LEVEL_WIDTH; }
    int getLevelHeight() const override { return LEVEL_HEIGHT; }
    Vector2 getSpawnTile() const override { return {2.5f, 8.5f}; }
    Rectangle getGoalTileArea() const override { return {32.0f, 8.5f, 3.0f, 2.5f}; }
    void renderForeground() override;
};

#endif // PLATFORM_LEVEL_ONE_H
