#include "Map.h"

Map::Map(int mapColumns, int mapRows, unsigned int *levelData,
         const char *textureFilePath, float tileSize, int textureColumns,
         int textureRows, Vector2 origin, Rectangle atlasRegion,
         const Texture2D *sharedTexture) :
         mMapColumns {mapColumns}, mMapRows {mapRows},
         mLevelData {levelData}, mTextureAtlas {},
         mTileSize {tileSize}, mTextureColumns {textureColumns},
         mTextureRows {textureRows}, mOrigin {origin},
         mLeftBoundary {0.0f}, mRightBoundary {0.0f},
         mTopBoundary {0.0f}, mBottomBoundary {0.0f},
         mAtlasRegion {atlasRegion},
         mUseAtlasRegion {atlasRegion.width > 0.0f && atlasRegion.height > 0.0f},
         mOwnsTexture {true}
{
    auto clampRegion = [](Rectangle region, int width, int height)
    {
        region.x = fmaxf(0.0f, fminf(region.x, static_cast<float>(width - 1)));
        region.y = fmaxf(0.0f, fminf(region.y, static_cast<float>(height - 1)));
        float maxWidth = static_cast<float>(width) - region.x;
        float maxHeight = static_cast<float>(height) - region.y;
        region.width  = fmaxf(0.0f, fminf(region.width,  maxWidth));
        region.height = fmaxf(0.0f, fminf(region.height, maxHeight));
        return region;
    };

    if (sharedTexture)
    {
        mTextureAtlas = *sharedTexture;
        mOwnsTexture = false;
    }
    else
    {
        mTextureAtlas = LoadTexture(textureFilePath);
    }

    if (mUseAtlasRegion)
    {
        mAtlasRegion = clampRegion(mAtlasRegion, mTextureAtlas.width, mTextureAtlas.height);
    }

    build();
}

Map::~Map()
{
    if (mOwnsTexture && mTextureAtlas.id > 0)
    {
        UnloadTexture(mTextureAtlas);
    }
}

void Map::build()
{
    mTextureAreas.clear();
    mTextureAreas.reserve(mTextureColumns * mTextureRows);

    // Calculate map boundaries in world coordinates
    mLeftBoundary   = mOrigin.x - (mMapColumns * mTileSize) / 2.0f;
    mRightBoundary  = mOrigin.x + (mMapColumns * mTileSize) / 2.0f;
    mTopBoundary    = mOrigin.y - (mMapRows * mTileSize) / 2.0f;
    mBottomBoundary = mOrigin.y + (mMapRows * mTileSize) / 2.0f;

    // Precompute texture areas for each tile
    const float atlasWidth = mUseAtlasRegion ? mAtlasRegion.width
                                             : static_cast<float>(mTextureAtlas.width);
    const float atlasHeight = mUseAtlasRegion ? mAtlasRegion.height
                                              : static_cast<float>(mTextureAtlas.height);
    const float atlasOffsetX = mUseAtlasRegion ? mAtlasRegion.x : 0.0f;
    const float atlasOffsetY = mUseAtlasRegion ? mAtlasRegion.y : 0.0f;

    for (int row = 0; row < mTextureRows; row++)
    {
        for (int col = 0; col < mTextureColumns; col++)
        {
            Rectangle textureArea = {
                atlasOffsetX + (static_cast<float>(col) * (atlasWidth / mTextureColumns)),
                atlasOffsetY + (static_cast<float>(row) * (atlasHeight / mTextureRows)),
                atlasWidth / mTextureColumns,
                atlasHeight / mTextureRows
            };

            mTextureAreas.push_back(textureArea);
        }
    }
}

void Map::refresh(unsigned int *levelData, int mapColumns, int mapRows, Vector2 origin)
{
    mLevelData = levelData;
    mMapColumns = mapColumns;
    mMapRows = mapRows;
    mOrigin = origin;
    build();
}

void Map::render()
{
    // Draw each tile in the map
    for (int row = 0; row < mMapRows; row++)
    {
        // Draw each column in the row
        for (int col = 0; col < mMapColumns; col++)
        {
            // Get the tile index at the current row and column
            int tile = mLevelData[row * mMapColumns + col];

            // If the tile index is 0, we do not draw anything
            if (tile == 0) continue;

            Rectangle destinationArea = {
                mLeftBoundary + col * mTileSize,
                mTopBoundary  + row * mTileSize, // y-axis is inverted
                mTileSize,
                mTileSize
            };

            // Draw the tile
            DrawTexturePro(
                mTextureAtlas,
                mTextureAreas[tile - 1], // -1 because tile indices start at 1
                destinationArea,
                {0.0f, 0.0f}, // origin
                0.0f,         // rotation
                WHITE         // tint
            );
        }
    }
}

bool Map::isSolidTileAt(Vector2 position)
{
    (void)position;
    // Ground is entirely walkable for now; trees/actors handle blocking.
    return false;

    /*
    // Original tile collision logic (kept for future solid-tile support)
    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
        return false;

    int tileXIndex = floor((position.x - mLeftBoundary) / mTileSize);
    int tileYIndex = floor((position.y - mTopBoundary) / mTileSize);

    if (tileXIndex < 0 || tileXIndex >= mMapColumns ||
        tileYIndex < 0 || tileYIndex >= mMapRows)
        return false;

    int tile = mLevelData[tileYIndex * mMapColumns + tileXIndex];
    return tile != 0;
    */
}

bool Map::isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap)
{
    *xOverlap = 0.0f;
    *yOverlap = 0.0f;
    (void)position;
    return false;

    /*
    // Original tile collision logic (kept for future solid-tile support)
    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
        return false;

    int tileXIndex = floor((position.x - mLeftBoundary) / mTileSize);
    int tileYIndex = floor((position.y - mTopBoundary) / mTileSize);

    if (tileXIndex < 0 || tileXIndex >= mMapColumns ||
        tileYIndex < 0 || tileYIndex >= mMapRows)
        return false;

    int tile = mLevelData[tileYIndex * mMapColumns + tileXIndex];
    if (tile == 0) return false;

    float tileCentreX = mLeftBoundary + tileXIndex * mTileSize + mTileSize / 2.0f;
    float tileCentreY = mTopBoundary + tileYIndex * mTileSize + mTileSize / 2.0f;

    *xOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.x - tileCentreX));
    *yOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.y - tileCentreY));
    return true;
    */
}
