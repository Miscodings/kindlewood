#include "cs3113.h"
#include <vector>

#ifndef MAP_H
#define MAP_H

struct Tileset {
    Texture2D texture;
    int firstGid;
    int tileCount;
    int columns;
};

struct MapLayer {
    std::vector<unsigned int> data;
    bool isCollidable;
};

class Map
{
private:
    int mMapColumns;
    int mMapRows;

    std::vector<MapLayer> mMapLayers;

    std::vector<Tileset> mTilesets;

    float mTileSize;
    Vector2 mOrigin; 
    float mLeftBoundary;
    float mRightBoundary;
    float mTopBoundary;
    float mBottomBoundary;

public:
    Map(int mapColumns, int mapRows, float tileSize, Vector2 origin);
    ~Map();

    void addTileset(const char* filepath, int firstGid);
    
    void addLayer(std::vector<unsigned int> layerData, bool isCollidable);

    void build();
    void render();
    bool isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap);

    int   getMapColumns() const { return mMapColumns; };
    int   getMapRows()    const { return mMapRows;    };
    float getTileSize()   const { return mTileSize;   };
    
    float getLeftBoundary()   const { return mLeftBoundary;   };
    float getRightBoundary()  const { return mRightBoundary;  };
    float getTopBoundary()    const { return mTopBoundary;    };
    float getBottomBoundary() const { return mBottomBoundary; };
};

#endif