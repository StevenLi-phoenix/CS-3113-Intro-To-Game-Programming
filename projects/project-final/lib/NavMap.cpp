#include "NavMap.h"
#include "Entity.h"
#include "../constants.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace
{
    constexpr int kNeighbourOffsets[8][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}
    };

    constexpr float kCardinalCost = 1.0f;
    constexpr float kDiagonalCost = 1.41421356f;

    float heuristicOctile(int ax, int ay, int bx, int by)
    {
        const float dx = static_cast<float>(std::abs(ax - bx));
        const float dy = static_cast<float>(std::abs(ay - by));
        const float minDiff = std::min(dx, dy);
        const float maxDiff = std::max(dx, dy);
        return (maxDiff - minDiff) * kCardinalCost + minDiff * kDiagonalCost;
    }
}

int NavMap::sPathBudgetRemaining = 0;
bool NavMap::sBudgetInitialised = false;
namespace
{
    struct PathStats
    {
        int aStarSuccess = 0;
        double aStarMs = 0.0;
        int aStarFail = 0;
        double aStarFailMs = 0.0;
        int bfsSuccess = 0;
        double bfsMs = 0.0;
        int bfsFail = 0;
        double bfsFailMs = 0.0;
        double lastLog = 0.0;

        void addAStar(bool success, double ms)
        {
            if (success)
            {
                ++aStarSuccess;
                aStarMs += ms;
            }
            else
            {
                ++aStarFail;
                aStarFailMs += ms;
            }
        }

        void addBfs(bool success, double ms)
        {
            if (success)
            {
                ++bfsSuccess;
                bfsMs += ms;
            }
            else
            {
                ++bfsFail;
                bfsFailMs += ms;
            }
        }

        void logIfNeeded()
        {
            const double now = GetTime();
            if (lastLog <= 0.0)
            {
                lastLog = now;
                return;
            }
            if ((now - lastLog) < 0.5)
            {
                return;
            }
            lastLog = now;
            if (aStarSuccess == 0 && aStarFail == 0 && bfsSuccess == 0 && bfsFail == 0)
            {
                return;
            }
            LOG_DEBUG(TextFormat("Nav path summary: A* ok=%d time=%.2fms fail=%d time=%.2fms | BFS ok=%d time=%.2fms fail=%d time=%.2fms",
                                 aStarSuccess,
                                 aStarMs,
                                 aStarFail,
                                 aStarFailMs,
                                 bfsSuccess,
                                 bfsMs,
                                 bfsFail,
                                 bfsFailMs));
            aStarSuccess = aStarFail = bfsSuccess = bfsFail = 0;
            aStarMs = aStarFailMs = bfsMs = bfsFailMs = 0.0;
        }
    };

    PathStats gNavPathStats;
}

void NavMap::beginFrame()
{
    beginFrame(pathfinding::REQUEST_BUDGET_PER_FRAME);
}

void NavMap::beginFrame(int budget)
{
    sPathBudgetRemaining = std::max(0, budget);
    sBudgetInitialised = true;
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
        mPaddedCache.clear();
        return;
    }

    mColumns = columns;
    mRows = rows;
    mTileSize = tileSize;
    mChunkStartX = chunkStartX;
    mChunkStartY = chunkStartY;
    mWalkable.assign(static_cast<size_t>(columns * rows), 1);
    mPaddedCache.clear();

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

bool NavMap::isWalkable(const std::vector<uint8_t> &grid, int gridX, int gridY) const
{
    if (gridX < 0 || gridX >= mColumns || gridY < 0 || gridY >= mRows)
    {
        return false;
    }

    const size_t index = static_cast<size_t>(gridY * mColumns + gridX);
    if (index >= grid.size())
    {
        return false;
    }
    return grid[index] != 0;
}

int NavMap::toIndex(int gridX, int gridY) const
{
    if (gridX < 0 || gridX >= mColumns || gridY < 0 || gridY >= mRows)
    {
        return -1;
    }
    return gridY * mColumns + gridX;
}

const std::vector<uint8_t>& NavMap::buildWalkableCopy(float clearanceRadius) const
{
    const int paddingTiles = static_cast<int>(std::ceil(std::max(clearanceRadius, 0.0f) / mTileSize));
    const int cacheKey = std::max(0, paddingTiles);
    auto it = mPaddedCache.find(cacheKey);
    if (it != mPaddedCache.end())
    {
        return it->second;
    }

    std::vector<uint8_t> grid = mWalkable;
    if (!grid.empty() && paddingTiles > 0)
    {
        applyBorderPadding(grid, paddingTiles);
        applyPadding(grid, paddingTiles);
    }
    auto inserted = mPaddedCache.emplace(cacheKey, std::move(grid));
    return inserted.first->second;
}

void NavMap::applyPadding(std::vector<uint8_t> &grid, int paddingTiles) const
{
    if (paddingTiles <= 0 || grid.empty())
    {
        return;
    }

    const int radiusSq = paddingTiles * paddingTiles;
    for (int y = 0; y < mRows; ++y)
    {
        for (int x = 0; x < mColumns; ++x)
        {
            const int baseIndex = toIndex(x, y);
            if (baseIndex < 0 || mWalkable[static_cast<size_t>(baseIndex)] != 0)
            {
                continue;
            }

            for (int ny = std::max(0, y - paddingTiles); ny <= std::min(mRows - 1, y + paddingTiles); ++ny)
            {
                for (int nx = std::max(0, x - paddingTiles); nx <= std::min(mColumns - 1, x + paddingTiles); ++nx)
                {
                    const int dx = x - nx;
                    const int dy = y - ny;
                    if (dx * dx + dy * dy > radiusSq)
                    {
                        continue;
                    }
                    const int nIndex = toIndex(nx, ny);
                    if (nIndex >= 0 && static_cast<size_t>(nIndex) < grid.size())
                    {
                        grid[static_cast<size_t>(nIndex)] = 0;
                    }
                }
            }
        }
    }
}

void NavMap::applyBorderPadding(std::vector<uint8_t> &grid, int paddingTiles) const
{
    if (paddingTiles <= 0 || grid.empty())
    {
        return;
    }

    for (int y = 0; y < mRows; ++y)
    {
        for (int x = 0; x < mColumns; ++x)
        {
            if (x < paddingTiles || y < paddingTiles ||
                x >= mColumns - paddingTiles || y >= mRows - paddingTiles)
            {
                const int index = toIndex(x, y);
                if (index >= 0 && static_cast<size_t>(index) < grid.size())
                {
                    grid[static_cast<size_t>(index)] = 0;
                }
            }
        }
    }
}

std::vector<float> NavMap::buildSoftCostMap(const std::vector<std::vector<Vector2>> &softReservations) const
{
    std::vector<float> costs(static_cast<size_t>(mColumns * mRows), 0.0f);
    if (softReservations.empty())
    {
        return costs;
    }

    for (const auto &path : softReservations)
    {
        for (size_t i = 0; i < path.size(); ++i)
        {
            int gx = 0;
            int gy = 0;
            if (!worldToGrid(path[i], gx, gy))
            {
                continue;
            }
            const int idx = toIndex(gx, gy);
            if (idx < 0)
            {
                continue;
            }

            const float weight = pathfinding::SOFT_RESERVATION_BASE_COST /
                                 std::max(1.0f, std::log2(static_cast<float>(i) + 2.0f));
            costs[static_cast<size_t>(idx)] += weight;
        }
    }
    return costs;
}

bool NavMap::consumeBudget(bool *throttled) const
{
    if (!sBudgetInitialised)
    {
        sPathBudgetRemaining = pathfinding::REQUEST_BUDGET_PER_FRAME;
        sBudgetInitialised = true;
    }

    if (sPathBudgetRemaining <= 0)
    {
        if (throttled)
        {
            *throttled = true;
        }
        return false;
    }

    --sPathBudgetRemaining;
    return true;
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

std::vector<Vector2> NavMap::findPathAStar(const std::vector<uint8_t> &grid,
                                           const std::vector<float> *softCosts,
                                           int startIndex,
                                           int goalIndex) const
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
    fScore[static_cast<size_t>(startIndex)] = heuristicOctile(startX, startY, goalX, goalY);

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
            if (neighbourIndex < 0 || !isWalkable(grid, neighbourX, neighbourY))
            {
                continue;
            }

            const bool isDiagonalStep = (offset[0] != 0) && (offset[1] != 0);
            if (isDiagonalStep)
            {
                const int adjXIndex = toIndex(currentX + offset[0], currentY);
                const int adjYIndex = toIndex(currentX, currentY + offset[1]);
                if (adjXIndex < 0 || adjYIndex < 0 ||
                    !isWalkable(grid, currentX + offset[0], currentY) ||
                    !isWalkable(grid, currentX, currentY + offset[1]))
                {
                    continue;
                }
            }

            const float stepCost = isDiagonalStep ? kDiagonalCost : kCardinalCost;
            const float reservationCost = softCosts ? (*softCosts)[static_cast<size_t>(neighbourIndex)] : 0.0f;
            const float tentativeG = gScore[static_cast<size_t>(current.index)] + stepCost + reservationCost;
            if (tentativeG >= gScore[static_cast<size_t>(neighbourIndex)])
            {
                continue;
            }

            cameFrom[static_cast<size_t>(neighbourIndex)] = current.index;
            gScore[static_cast<size_t>(neighbourIndex)] = tentativeG;
            const float h = heuristicOctile(neighbourX, neighbourY, goalX, goalY);
            fScore[static_cast<size_t>(neighbourIndex)] = tentativeG + h;
            openSet.push({neighbourIndex, fScore[static_cast<size_t>(neighbourIndex)], tentativeG});
        }
    }

    return {};
}

std::vector<Vector2> NavMap::findPathBFS(const std::vector<uint8_t> &grid,
                                         int startIndex,
                                         int goalIndex) const
{
    const int totalNodes = mColumns * mRows;
    if (startIndex < 0 || goalIndex < 0 || totalNodes <= 0)
    {
        return {};
    }

    std::vector<int> cameFrom(static_cast<size_t>(totalNodes), -1);
    std::vector<uint8_t> visited(static_cast<size_t>(totalNodes), 0);
    std::queue<int> frontier;
    frontier.push(startIndex);
    visited[static_cast<size_t>(startIndex)] = 1;

    while (!frontier.empty())
    {
        const int current = frontier.front();
        frontier.pop();

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
            if (neighbourIndex < 0 || !isWalkable(grid, neighbourX, neighbourY))
            {
                continue;
            }

            if (visited[static_cast<size_t>(neighbourIndex)])
            {
                continue;
            }

            visited[static_cast<size_t>(neighbourIndex)] = 1;
            cameFrom[static_cast<size_t>(neighbourIndex)] = current;
            frontier.push(neighbourIndex);
        }
    }

    return {};
}

void NavMap::applyStaticObstacles(const std::vector<Entity*> &obstacles, float padding)
{
    if (!hasData() || mWalkable.empty())
    {
        return;
    }

    const float pad = std::max(0.0f, padding);
    for (Entity *entity : obstacles)
    {
        if (!entity || !entity->getIsActive() || !entity->getCanCollide())
        {
            continue;
        }

        const Vector2 collider = entity->getColliderDimensions();
        const Vector2 pos = entity->getPosition();
        const float halfW = 0.5f * collider.x + pad;
        const float halfH = 0.5f * collider.y + pad;
        const float minX = pos.x - halfW;
        const float maxX = pos.x + halfW;
        const float minY = pos.y - halfH;
        const float maxY = pos.y + halfH;

        const int minGridX = static_cast<int>(std::floor(minX / mTileSize)) - mChunkStartX;
        const int maxGridX = static_cast<int>(std::floor(maxX / mTileSize)) - mChunkStartX;
        const int minGridY = static_cast<int>(std::floor(minY / mTileSize)) - mChunkStartY;
        const int maxGridY = static_cast<int>(std::floor(maxY / mTileSize)) - mChunkStartY;

        const int clampedMinX = std::max(0, minGridX);
        const int clampedMaxX = std::min(mColumns - 1, maxGridX);
        const int clampedMinY = std::max(0, minGridY);
        const int clampedMaxY = std::min(mRows - 1, maxGridY);

        for (int y = clampedMinY; y <= clampedMaxY; ++y)
        {
            for (int x = clampedMinX; x <= clampedMaxX; ++x)
            {
                const int index = toIndex(x, y);
                if (index >= 0 && static_cast<size_t>(index) < mWalkable.size())
                {
                    mWalkable[static_cast<size_t>(index)] = 0;
                }
            }
        }
    }
    mPaddedCache.clear();
}

std::vector<Vector2> NavMap::findPath(const Vector2 &worldStart, const Vector2 &worldGoal) const
{
    static const std::vector<std::vector<Vector2>> kEmptyReservations;
    return findPath(worldStart, worldGoal, 0.0f, kEmptyReservations, nullptr);
}

std::vector<Vector2> NavMap::findPath(const Vector2 &worldStart,
                                      const Vector2 &worldGoal,
                                      float clearanceRadius,
                                      const std::vector<std::vector<Vector2>> &softReservations,
                                      bool *throttled) const
{
    if (throttled)
    {
        *throttled = false;
    }

    if (!hasData())
    {
        return {};
    }

    if (!consumeBudget(throttled))
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

    const std::vector<uint8_t> &workingGrid = buildWalkableCopy(clearanceRadius);
    if (workingGrid.empty())
    {
        return {};
    }

    auto findNearestWalkable = [&](int gx, int gy, int maxRadius, int &outX, int &outY) -> bool
    {
        if (isWalkable(workingGrid, gx, gy))
        {
            outX = gx;
            outY = gy;
            return true;
        }

        for (int radius = 1; radius <= maxRadius; ++radius)
        {
            for (int dy = -radius; dy <= radius; ++dy)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    if (std::abs(dx) != radius && std::abs(dy) != radius)
                    {
                        continue;
                    }
                    const int nx = gx + dx;
                    const int ny = gy + dy;
                    if (isWalkable(workingGrid, nx, ny))
                    {
                        outX = nx;
                        outY = ny;
                        return true;
                    }
                }
            }
        }
        return false;
    };

    const int searchRadius = 6;
    if (!findNearestWalkable(startX, startY, searchRadius, startX, startY) ||
        !findNearestWalkable(goalX, goalY, searchRadius, goalX, goalY))
    {
        return {};
    }

    const int startIndex = toIndex(startX, startY);
    const int goalIndex = toIndex(goalX, goalY);

    const bool hasSoftCosts = !softReservations.empty();
    const std::vector<float> softCosts = hasSoftCosts ? buildSoftCostMap(softReservations)
                                                      : std::vector<float>();
    const std::vector<float> *softPtr = hasSoftCosts ? &softCosts : nullptr;

    const double tStart = GetTime();
    bool usedAStar = true;
    auto path = findPathAStar(workingGrid, softPtr, startIndex, goalIndex);
    if (!path.empty())
    {
        if (isDebugMode())
        {
            const double elapsedMs = (GetTime() - tStart) * 1000.0;
            gNavPathStats.addAStar(true, elapsedMs);
            gNavPathStats.logIfNeeded();
        }
        return path;
    }

    usedAStar = false;
    path = findPathBFS(workingGrid, startIndex, goalIndex);
    const double elapsedMs = (GetTime() - tStart) * 1000.0;
    if (isDebugMode())
    {
        gNavPathStats.addBfs(!path.empty(), elapsedMs);
        gNavPathStats.logIfNeeded();
        if (!path.empty())
        {
            cacheDebugPath(path, usedAStar ? RED : BLUE);
        }
    }
    return path;
}

void NavMap::cacheDebugPath(const std::vector<Vector2> &path, Color color) const
{
    if (path.size() < 2)
    {
        return;
    }

    const size_t maxPaths = 64;
    if (mDebugPaths.size() >= maxPaths)
    {
        mDebugPaths.erase(mDebugPaths.begin());
    }

    DebugPath entry;
    entry.nodes = path;
    entry.color = color;
    mDebugPaths.push_back(std::move(entry));
}

void NavMap::addDebugPath(const std::vector<Vector2> &path, Color color) const
{
    cacheDebugPath(path, color);
}

void NavMap::clearDebugPaths() const
{
    mDebugPaths.clear();
}

void NavMap::debugRender() const
{
    if (!isDebugMode() || mDebugPaths.empty())
    {
        return;
    }

    const float thickness = 2.0f;
    for (const DebugPath &entry : mDebugPaths)
    {
        if (entry.nodes.size() < 2)
        {
            continue;
        }
        for (size_t i = 1; i < entry.nodes.size(); ++i)
        {
            DrawLineEx(entry.nodes[i - 1], entry.nodes[i], thickness, entry.color);
        }
    }
    mDebugPaths.clear();
}
