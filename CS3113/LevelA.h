#ifndef LEVELA_H
#define LEVELA_H

#include "Scene.h"

class LevelA : public Scene {
public:
    static constexpr int LEVEL_WIDTH = 32,
                         LEVEL_HEIGHT = 8;
    
private:
    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        34,  0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 34,
        15,  0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 15,
        15,  0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0, 52, 54, 0,  0,  0,  0,  0,  0, 15,
        15,  0,  0,  0, 0, 52, 54, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,  0,  0, 0, 52, 54, 0,  0,  0, 0,  0,  0,  0,  0,  0, 15,
        15,  0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0, 52, 54, 0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 15,
        18,  2,  2,  3, 0,  0,  0, 0,  1,  2,  2,  2,  3, 0, 52, 54, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 13,
        18, 11, 11, 12, 0,  0,  0, 0, 10, 11, 11, 11, 12, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0, 52, 53, 53, 53, 53, 12,
        18, 11, 11, 12, 0,  0,  0, 0, 10, 11, 11, 11, 12, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0, 0,  0,  0,  0,  0,  0, 13
    };

public:
    static constexpr float TILE_DIMENSION       = 75.0f,
                        END_GAME_THRESHOLD      = 800.0f;

    LevelA();
    LevelA(Vector2 origin, const char *bgHexCode);
    ~LevelA();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif