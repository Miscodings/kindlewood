#include "Map.h"
#include <cmath>

Map::Map(int mapColumns, int mapRows, float tileSize, Vector2 origin)
    : mMapColumns(mapColumns), mMapRows(mapRows),
      mTileSize(tileSize), mOrigin(origin)
{
    build();
}

Map::~Map()
{
    for (Tileset &ts : mTilesets) UnloadTexture(ts.texture);
}

void Map::addTileset(const char *filepath, int firstGid)
{
    Texture2D tex = LoadTexture(filepath);
    int columns = tex.width / (int)mTileSize;
    int rows = tex.height / (int)mTileSize;
    
    Tileset newSet;
    newSet.texture = tex;
    newSet.firstGid = firstGid;
    newSet.columns = columns;
    newSet.tileCount = columns * rows;
    mTilesets.push_back(newSet);
}

void Map::addLayer(std::vector<unsigned int> layerData, bool isCollidable)
{
    MapLayer newLayer;
    newLayer.data = layerData;
    newLayer.isCollidable = isCollidable;
    mMapLayers.push_back(newLayer);
}

void Map::build()
{
    mLeftBoundary   = mOrigin.x - (mMapColumns * mTileSize) / 2.0f;
    mRightBoundary  = mOrigin.x + (mMapColumns * mTileSize) / 2.0f;
    mTopBoundary    = mOrigin.y - (mMapRows * mTileSize) / 2.0f;
    mBottomBoundary = mOrigin.y + (mMapRows * mTileSize) / 2.0f;
}

void Map::render()
{
    for (const auto& layer : mMapLayers)
    {
        for (int row = 0; row < mMapRows; row++)
        {
            for (int col = 0; col < mMapColumns; col++)
            {
                int globalTileID = layer.data[row * mMapColumns + col];

                if (globalTileID == 0) continue;

                Tileset *activeTileset = nullptr;
                int maxGidFound = -1;

                for (Tileset &ts : mTilesets) {
                    if (globalTileID >= ts.firstGid) {
                        if (ts.firstGid > maxGidFound) {
                            maxGidFound = ts.firstGid;
                            activeTileset = &ts;
                        }
                    }
                }

                if (activeTileset != nullptr) {
                    int localID = globalTileID - activeTileset->firstGid;
                    int texCol = localID % activeTileset->columns;
                    int texRow = localID / activeTileset->columns;

                    Rectangle sourceRec = {
                        (float)texCol * mTileSize, (float)texRow * mTileSize,
                        mTileSize, mTileSize
                    };

                    Rectangle destRec = {
                        mLeftBoundary + col * mTileSize,
                        mTopBoundary + row * mTileSize,
                        mTileSize, mTileSize
                    };

                    DrawTexturePro(activeTileset->texture, sourceRec, destRec, {0,0}, 0.0f, WHITE);
                }
            }
        }
    }
}

bool Map::isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap)
{
    *xOverlap = 0.0f; *yOverlap = 0.0f;

    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
    {
        return true; 
    }

    int tileX = floor((position.x - mLeftBoundary) / mTileSize);
    int tileY = floor((position.y - mTopBoundary) / mTileSize);

    if (tileX < 0 || tileX >= mMapColumns || tileY < 0 || tileY >= mMapRows) return false;

    bool hitSolid = false;
    for (const auto& layer : mMapLayers)
    {
        if (!layer.isCollidable) continue;

        int tile = layer.data[tileY * mMapColumns + tileX];
        if (tile != 0) {
            hitSolid = true;
            break;
        }
    }

    if (!hitSolid) return false;

    float tileCentreX = mLeftBoundary + tileX * mTileSize + mTileSize / 2.0f;
    float tileCentreY = mTopBoundary + tileY * mTileSize + mTileSize / 2.0f;

    *xOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.x - tileCentreX));
    *yOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.y - tileCentreY));

    return true;
}