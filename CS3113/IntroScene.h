#ifndef INTROSCENE_H
#define INTROSCENE_H

#include "Scene.h"
#include <vector>
#include <string>

class IntroScene : public Scene {
public:
    IntroScene(Vector2 origin, const char* bgHexCode);
    ~IntroScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    std::vector<std::string> mStoryLines;
    int mCurrentLineIndex = 0;
    
    std::string mDisplayedText = "";
    int mCharIndex = 0;
    float mTypewriterTimer = 0.0f;
    
    bool mIsFinished = false;
};

#endif