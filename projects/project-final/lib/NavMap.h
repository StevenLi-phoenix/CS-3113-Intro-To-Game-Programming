#ifndef NAVMAP_H
#define NAVMAP_H

#include "Helper.h"
#include <vector>
#include <cstdint>
#include <unordered_map>

class Entity;

class NavMap
{
public:
    void build(const unsigned int *tiles,
               int columns,
               int rows,
               float tileSize,
               int chunkStartX,
               int chunkStartY);

    bool hasData() const { return mColumns > 0 && mRows > 0 && !mWalkable.empty(); }
    bool worldToGrid(const Vector2 &worldPos, int &gridX, int &gridY) const;
    Vector2 gridToWorld(int gridX, int gridY) const;
    std::vector<Vector2> findPath(const Vector2 &worldStart,
                                  const Vector2 &worldGoal) const;
    std::vector<Vector2> findPath(const Vector2 &worldStart,
                                  const Vector2 &worldGoal,
                                  float clearanceRadius,
                                  const std::vector<std::vector<Vector2>> &softReservations,
                                  bool *throttled = nullptr) const;
    void applyStaticObstacles(const std::vector<Entity*> &obstacles, float padding = 0.0f);
    static void beginFrame();
    static void beginFrame(int budget);
    void addDebugPath(const std::vector<Vector2> &path, Color color) const;
    void clearDebugPaths() const;
    void debugRender() const;

private:
    bool isWalkable(const std::vector<uint8_t> &grid, int gridX, int gridY) const;
    int toIndex(int gridX, int gridY) const;
    const std::vector<uint8_t>& buildWalkableCopy(float clearanceRadius) const;
    void applyPadding(std::vector<uint8_t> &grid, int paddingTiles) const;
    void applyBorderPadding(std::vector<uint8_t> &grid, int paddingTiles) const;
    std::vector<float> buildSoftCostMap(const std::vector<std::vector<Vector2>> &softReservations) const;
    bool consumeBudget(bool *throttled) const;
    std::vector<Vector2> reconstructPath(const std::vector<int> &cameFrom, int currentIndex) const;
    std::vector<Vector2> findPathAStar(const std::vector<uint8_t> &grid,
                                       const std::vector<float> *softCosts,
                                       int startIndex,
                                       int goalIndex) const;
    std::vector<Vector2> findPathBFS(const std::vector<uint8_t> &grid,
                                     int startIndex,
                                     int goalIndex) const;
    void cacheDebugPath(const std::vector<Vector2> &path, Color color) const;

    struct DebugPath
    {
        std::vector<Vector2> nodes;
        Color color;
    };

    std::vector<uint8_t> mWalkable;
    int mColumns = 0;
    int mRows = 0;
    float mTileSize = 1.0f;
    int mChunkStartX = 0;
    int mChunkStartY = 0;
    mutable std::vector<DebugPath> mDebugPaths;
    mutable std::unordered_map<int, std::vector<uint8_t>> mPaddedCache;
    static int sPathBudgetRemaining;
    static bool sBudgetInitialised;
};

#endif // NAVMAP_H
