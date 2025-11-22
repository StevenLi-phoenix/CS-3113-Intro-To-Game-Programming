#ifndef NAVMAP_H
#define NAVMAP_H

#include "Helper.h"
#include <vector>
#include <cstdint>

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
    std::vector<Vector2> findPath(const Vector2 &worldStart, const Vector2 &worldGoal) const;
    void addDebugPath(const std::vector<Vector2> &path, Color color) const;
    void clearDebugPaths() const;
    void debugRender() const;

private:
    bool isWalkable(int gridX, int gridY) const;
    int toIndex(int gridX, int gridY) const;
    std::vector<Vector2> reconstructPath(const std::vector<int> &cameFrom, int currentIndex) const;
    std::vector<Vector2> findPathAStar(int startIndex, int goalIndex) const;
    std::vector<Vector2> findPathDFS(int startIndex, int goalIndex) const;
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
};

#endif // NAVMAP_H

