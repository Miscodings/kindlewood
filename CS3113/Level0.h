#ifndef LEVEL0_H
#define LEVEL0_H

#include "Scene.h"

class Level0 : public Scene {
public:
    static constexpr int LEVEL_WIDTH = 14,
                         LEVEL_HEIGHT = 8;
private:
    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
    };

public:
    static constexpr float TILE_DIMENSION = 75.0f;
    Texture2D mBackgroundTexture;

    Level0();
    Level0(Vector2 origin, const char *bgHexCode);
    ~Level0();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif