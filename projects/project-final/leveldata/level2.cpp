#include "level2.h"
#include "attack_enemy.h"
#include <algorithm>

namespace
{
    struct ShooterSpawnSettings
    {
        unsigned int salt = 0u;
        float spawnThreshold = 0.0f;
        int spacing = 1;
        float minHeightPx = 0.0f;
        float maxHeightPx = 0.0f;
        float maxDistanceScale = 1.0f;
    };

    constexpr ShooterSpawnSettings SHOOTER_SPAWN_SETTINGS{
        0x9e3779b9u,
        0.9f,
        4,
        44.0f,
        68.0f,
        0.9f
    };
}

float Level2::enemyGoldDropChance() const
{
    return 0.8f;
}

std::unique_ptr<Scene> Level2::createNextSceneAfterBoss()
{
    return nullptr;
}

void Level2::spawnEnemiesForChunk(const std::pair<int, int> &chunk,
                                  std::vector<Enemy*> &bucket,
                                  const Vector2 &playerPos)
{
    const int spacing = std::max(SHOOTER_SPAWN_SETTINGS.spacing, 1);
    const int chunkSize = getChunkSize();
    const int chunkTileStartX = chunk.first * chunkSize;
    const int chunkTileStartY = chunk.second * chunkSize;
    const float chunkWorldSize = static_cast<float>(chunkSize) * getTileSize();
    const float maxSpawnDistance = chunkWorldSize * SHOOTER_SPAWN_SETTINGS.maxDistanceScale;
    const float maxSpawnDistanceSq = maxSpawnDistance * maxSpawnDistance;

    const MapGenerator &gen = getMapGenerator();

    int spawnedCount = 0;
    for (int row = 0; row < chunkSize; row += spacing)
    {
        for (int col = 0; col < chunkSize; col += spacing)
        {
            const int worldX = chunkTileStartX + col;
            const int worldY = chunkTileStartY + row;

            float spawnNoise = gen.whiteNoise(worldX, worldY, SHOOTER_SPAWN_SETTINGS.salt);
            if (spawnNoise < SHOOTER_SPAWN_SETTINGS.spawnThreshold)
            {
                continue;
            }

            const float variantNoise = gen.whiteNoise(worldX, worldY, SHOOTER_SPAWN_SETTINGS.salt + 1u);
            const float heightNoise = gen.whiteNoise(worldX, worldY, SHOOTER_SPAWN_SETTINGS.salt + 2u);

            int variant = static_cast<int>(variantNoise * 3.0f);
            variant = std::clamp(variant, 0, 2);

            const float shooterHeight = SHOOTER_SPAWN_SETTINGS.minHeightPx +
                                        heightNoise * (SHOOTER_SPAWN_SETTINGS.maxHeightPx - SHOOTER_SPAWN_SETTINGS.minHeightPx);

            const float worldPosX = (static_cast<float>(worldX) + 0.5f) * getTileSize();
            const float worldPosY = (static_cast<float>(worldY) + 0.5f) * getTileSize();

            const float dx = worldPosX - playerPos.x;
            const float dy = worldPosY - playerPos.y;
            if ((dx * dx + dy * dy) > maxSpawnDistanceSq)
            {
                continue;
            }

            AttackEnemy *attacker = new AttackEnemy({worldPosX, worldPosY},
                                                    variant,
                                                    &getSpreadProjectiles(),
                                                    shooterHeight);
            attacker->setNavMap(getNavMap());
            bucket.push_back(attacker);
            spawnedCount++;
        }
    }

    if (spawnedCount > 0 && isDebugMode())
    {
        LOG_DEBUG(TextFormat("Level2 chunk (%d,%d) spawned %d shooters",
                             chunk.first,
                             chunk.second,
                             spawnedCount));
    }
}
