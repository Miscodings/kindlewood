#include "IntroScene.h"

IntroScene::IntroScene(Vector2 origin, const char* bgHexCode) 
    : Scene(origin, bgHexCode) {}

IntroScene::~IntroScene() {}

void IntroScene::initialise()
{
    mGameState.nextSceneID = -1;
    
    mStoryLines = {
        "Dear Friend...",
        "It has been too long since I've seen you.",
        "I heard you were looking for a change of pace.",
        "A place where the breeze smells like apples...",
        "And the rivers sparkle in the morning sun.",
        "Welcome to Kindlewood Sanctuary.",
        "It's a little overgrown right now, I admit.",
        "But with a little love, it could be home.",
        "The bus is arriving now...",
        "Are you ready to say hello?"
    };

    mCurrentLineIndex = 0;
    mCharIndex = 0;
    mDisplayedText = "";
}

void IntroScene::update(float deltaTime)
{
    if (mCharIndex < mStoryLines[mCurrentLineIndex].length()) 
    {
        mTypewriterTimer += deltaTime;
        if (mTypewriterTimer > 0.04f)
        {
            mDisplayedText += mStoryLines[mCurrentLineIndex][mCharIndex];
            mCharIndex++;
            mTypewriterTimer = 0.0f;
        }
    }

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) 
    {
        if (mCharIndex < mStoryLines[mCurrentLineIndex].length()) {
            mDisplayedText = mStoryLines[mCurrentLineIndex];
            mCharIndex = mStoryLines[mCurrentLineIndex].length();
        }
        else {
            mCurrentLineIndex++;
            if (mCurrentLineIndex >= mStoryLines.size()) {
                mIsFinished = true;
            } else {
                mDisplayedText = "";
                mCharIndex = 0;
            }
        }
    }

    if (mIsFinished) {
        mGameState.nextSceneID = 1; // Go to LevelA
    }
}

void IntroScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode)); 

    int boxWidth = 800;
    int boxHeight = 200;
    int xPos = (GetScreenWidth() - boxWidth) / 2;
    int yPos = (GetScreenHeight() - boxHeight) / 2;

    DrawRectangle(xPos, yPos, boxWidth, boxHeight, Fade(DARKGRAY, 0.5f)); 
    DrawRectangleLines(xPos, yPos, boxWidth, boxHeight, WHITE); // Add a border

    int fontSize = 30;
    int textWidth = MeasureText(mDisplayedText.c_str(), fontSize);
    int textX = (GetScreenWidth() - textWidth) / 2;
    int textY = yPos + (boxHeight / 2) - 15;

    DrawText(mDisplayedText.c_str(), textX, textY, fontSize, WHITE);
    
    if (mCharIndex >= mStoryLines[mCurrentLineIndex].length()) {
        if ((int)GetTime() % 2 == 0) {
            DrawText("PRESS SPACE >>", xPos + boxWidth - 250, yPos + boxHeight - 30, 20, LIGHTGRAY);
        }
    }
}

void IntroScene::shutdown() {}