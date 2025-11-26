#ifndef LEVEL2_H
#define LEVEL2_H

#include "level1.h"

class Level2 : public Level1
{
public:
    Level2() = default;
    ~Level2() override = default;

protected:
    void spawnEnemiesForChunk(const std::pair<int, int> &chunk,
                              std::vector<Enemy*> &bucket,
                              const Vector2 &playerPos) override;
    float enemyGoldDropChance() const override;
    std::unique_ptr<Scene> createNextSceneAfterBoss() override;
};

#endif // LEVEL2_H
