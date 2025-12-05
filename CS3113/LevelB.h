#ifndef LEVELB_H
#define LEVELB_H

#include "Scene.h"

class LevelB : public Scene {
public:
    static constexpr int LEVEL_WIDTH = 7;
    static constexpr int LEVEL_HEIGHT = 7;
private:
    unsigned int floor[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        174,748,748,748,748,748,181,
        174,798,798,798,798,798,181,
        174, 88, 88, 88, 88, 88,181,
        174, 88, 88, 88, 88, 88,181,
        174, 88, 88, 88, 88, 88,181,
        174, 88, 88, 88, 88, 88,181,
        480,981,981, 88,981,981,437
    };

    unsigned int walls[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        174,748,748,748,748,748,181,
        174,798,798,798,798,798,181,
        174,  0,  0,  0,  0,  0,181,
        174,  0,  0,  0,  0,  0,181,
        174,  0,  0,  0,  0,  0,181,
        174,  0,  0,  0,  0,  0,181,
        480,981,981,  0,981,981,437
    };

    bool mIsChatting = false;
    std::string mCurrentChatText = "";

public:
    static constexpr float TILE_DIMENSION = 32.0f;
    Texture2D mBackgroundTexture;

    LevelB();
    LevelB(Vector2 origin, const char *bgHexCode);
    ~LevelB();

    void setChat(const std::string& text) override {
        mIsChatting = true;
        mCurrentChatText = text;
    }

    bool isChatting() override { return mIsChatting; }
    void stopChat() override { mIsChatting = false; }

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
