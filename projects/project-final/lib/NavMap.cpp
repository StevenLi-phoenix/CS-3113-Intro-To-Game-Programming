#include "NavMap.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stack>

namespace
{
    constexpr int kNeighbourOffsets[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };

    float heuristicManhattan(int ax, int ay, int bx, int by)
    {
        return static_cast<float>(std::abs(ax - bx) + std::abs(ay - by));
    }
}

void NavMap::build(const unsigned int *tiles,
                   int columns,
                   int rows,
                   float tileSize,
                   int chunkStartX,
                   int chunkStartY)
{
    if (!tiles || columns <= 0 || rows <= 0 || tileSize <= 0.0f)
    {
        mWalkable.clear();
        mColumns = 0;
        mRows = 0;
        return;
    }

    mColumns = columns;
    mRows = rows;
    mTileSize = tileSize;
    mChunkStartX = chunkStartX;
    mChunkStartY = chunkStartY;
    mWalkable.assign(static_cast<size_t>(columns * rows), 1);

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < columns; ++col)
        {
            (void)tiles[row * columns + col];
            mWalkable[row * columns + col] = 1;
        }
    }
}

bool NavMap::worldToGrid(const Vector2 &worldPos, int &gridX, int &gridY) const
{
    if (!hasData())
    {
        return false;
    }

    const int tileX = static_cast<int>(std::floor(worldPos.x / mTileSize));
    const int tileY = static_cast<int>(std::floor(worldPos.y / mTileSize));

    gridX = tileX - mChunkStartX;
    gridY = tileY - mChunkStartY;

    if (gridX < 0 || gridX >= mColumns || gridY < 0 || gridY >= mRows)
    {
        return false;
    }

    return true;
}

Vector2 NavMap::gridToWorld(int gridX, int gridY) const
{
    const float worldX = (static_cast<float>(mChunkStartX + gridX) + 0.5f) * mTileSize;
    const float worldY = (static_cast<float>(mChunkStartY + gridY) + 0.5f) * mTileSize;
    return {worldX, worldY};
}

bool NavMap::isWalkable(int gridX, int gridY) const
{
    if (gridX < 0 || gridX >= mColumns || gridY < 0 || gridY >= mRows)
    {
        return false;
    }

    return mWalkable[static_cast<size_t>(gridY * mColumns + gridX)] != 0;
}

int NavMap::toIndex(int gridX, int gridY) const
{
    if (gridX < 0 || gridX >= mColumns || gridY < 0 || gridY >= mRows)
    {
        return -1;
    }
    return gridY * mColumns + gridX;
}

std::vector<Vector2> NavMap::reconstructPath(const std::vector<int> &cameFrom, int currentIndex) const
{
    std::vector<Vector2> path;
    if (currentIndex < 0)
    {
        return path;
    }

    int index = currentIndex;
    while (index >= 0)
    {
        const int gridX = index % mColumns;
        const int gridY = index / mColumns;
        path.push_back(gridToWorld(gridX, gridY));
        index = cameFrom[static_cast<size_t>(index)];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Vector2> NavMap::findPathAStar(int startIndex, int goalIndex) const
{
    const int totalNodes = mColumns * mRows;
    if (startIndex < 0 || goalIndex < 0 || totalNodes <= 0)
    {
        return {};
    }

    struct Node
    {
        int index;
        float fScore;
        float gScore;
    };

    auto cmp = [](const Node &a, const Node &b) { return a.fScore > b.fScore; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> openSet(cmp);

    const float kInf = std::numeric_limits<float>::infinity();
    std::vector<float> gScore(static_cast<size_t>(totalNodes), kInf);
    std::vector<float> fScore(static_cast<size_t>(totalNodes), kInf);
    std::vector<int> cameFrom(static_cast<size_t>(totalNodes), -1);
    std::vector<uint8_t> closed(static_cast<size_t>(totalNodes), 0);

    gScore[static_cast<size_t>(startIndex)] = 0.0f;
    const int startX = startIndex % mColumns;
    const int startY = startIndex / mColumns;
    const int goalX = goalIndex % mColumns;
    const int goalY = goalIndex / mColumns;
    fScore[static_cast<size_t>(startIndex)] = heuristicManhattan(startX, startY, goalX, goalY);

    openSet.push({startIndex, fScore[static_cast<size_t>(startIndex)], 0.0f});

    while (!openSet.empty())
    {
        const Node current = openSet.top();
        openSet.pop();

        if (closed[static_cast<size_t>(current.index)])
        {
            continue;
        }
        closed[static_cast<size_t>(current.index)] = 1;

        if (current.index == goalIndex)
        {
            return reconstructPath(cameFrom, goalIndex);
        }

        const int currentX = current.index % mColumns;
        const int currentY = current.index / mColumns;

        for (const auto &offset : kNeighbourOffsets)
        {
            const int neighbourX = currentX + offset[0];
            const int neighbourY = currentY + offset[1];
            const int neighbourIndex = toIndex(neighbourX, neighbourY);
            if (neighbourIndex < 0 || !isWalkable(neighbourX, neighbourY))
            {
                continue;
            }

            const float tentativeG = gScore[static_cast<size_t>(current.index)] + 1.0f;
            if (tentativeG >= gScore[static_cast<size_t>(neighbourIndex)])
            {
                continue;
            }

            cameFrom[static_cast<size_t>(neighbourIndex)] = current.index;
            gScore[static_cast<size_t>(neighbourIndex)] = tentativeG;
            const float h = heuristicManhattan(neighbourX, neighbourY, goalX, goalY);
            fScore[static_cast<size_t>(neighbourIndex)] = tentativeG + h;
            openSet.push({neighbourIndex, fScore[static_cast<size_t>(neighbourIndex)], tentativeG});
        }
    }

    return {};
}

std::vector<Vector2> NavMap::findPathDFS(int startIndex, int goalIndex) const
{
    const int totalNodes = mColumns * mRows;
    if (startIndex < 0 || goalIndex < 0 || totalNodes <= 0)
    {
        return {};
    }

    std::vector<int> cameFrom(static_cast<size_t>(totalNodes), -1);
    std::vector<uint8_t> visited(static_cast<size_t>(totalNodes), 0);
    std::stack<int> stack;
    stack.push(startIndex);
    visited[static_cast<size_t>(startIndex)] = 1;

    while (!stack.empty())
    {
        const int current = stack.top();
        stack.pop();

        if (current == goalIndex)
        {
            return reconstructPath(cameFrom, current);
        }

        const int currentX = current % mColumns;
        const int currentY = current / mColumns;

        for (const auto &offset : kNeighbourOffsets)
        {
            const int neighbourX = currentX + offset[0];
            const int neighbourY = currentY + offset[1];
            const int neighbourIndex = toIndex(neighbourX, neighbourY);
            if (neighbourIndex < 0 || !isWalkable(neighbourX, neighbourY))
            {
                continue;
            }

            if (visited[static_cast<size_t>(neighbourIndex)])
            {
                continue;
            }

            visited[static_cast<size_t>(neighbourIndex)] = 1;
            cameFrom[static_cast<size_t>(neighbourIndex)] = current;
            stack.push(neighbourIndex);
        }
    }

    return {};
}

std::vector<Vector2> NavMap::findPath(const Vector2 &worldStart, const Vector2 &worldGoal) const
{
    if (!hasData())
    {
        return {};
    }

    int startX = 0;
    int startY = 0;
    int goalX = 0;
    int goalY = 0;

    if (!worldToGrid(worldStart, startX, startY) ||
        !worldToGrid(worldGoal, goalX, goalY))
    {
        return {};
    }

    if (!isWalkable(startX, startY) || !isWalkable(goalX, goalY))
    {
        return {};
    }

    const int startIndex = toIndex(startX, startY);
    const int goalIndex = toIndex(goalX, goalY);

    const double tStart = GetTime();
    bool usedAStar = true;
    auto path = findPathAStar(startIndex, goalIndex);
    if (!path.empty())
    {
        if (isDebugMode())
        {
            const double elapsedMs = (GetTime() - tStart) * 1000.0;
            LOG_DEBUG(TextFormat("NavMap path A* (%d,%d)->(%d,%d) nodes=%d took=%.2fms",
                                 startX,
                                 startY,
                                 goalX,
                                 goalY,
                                 static_cast<int>(path.size()),
                                 elapsedMs));
        }
        return path;
    }

    usedAStar = false;
    path = findPathDFS(startIndex, goalIndex);
    const double elapsedMs = (GetTime() - tStart) * 1000.0;
    if (isDebugMode())
    {
        LOG_DEBUG(TextFormat("NavMap path %s (%d,%d)->(%d,%d) %s nodes=%d took=%.2fms",
                             usedAStar ? "A*" : "DFS",
                             startX,
                             startY,
                             goalX,
                             goalY,
                             path.empty() ? "failed" : "ok",
                             static_cast<int>(path.size()),
                             elapsedMs));
    }
    return path;
}


