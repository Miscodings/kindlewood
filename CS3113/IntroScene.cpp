#include "IntroScene.h"

IntroScene::IntroScene(Vector2 origin, const char* bgHexCode) 
    : Scene(origin, bgHexCode) {}

IntroScene::~IntroScene() {}

void IntroScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.bgm1 = LoadMusicStream("assets/game/music_intro.mp3");
    mGameState.bgm2 = LoadMusicStream("assets/game/music_level2.wav");
    mGameState.sfx_1 = LoadSound("assets/game/sfx_click.mp3");
    SetMusicVolume(mGameState.bgm1, 0.7f);
    PlayMusicStream(mGameState.bgm1);
    PlayMusicStream(mGameState.bgm2);
    
    mStoryLines = {
        "Dear Friend...",
        "I heard you were looking for a change of pace.",
        "A place where the breeze smells like apples...",
        "And the rivers sparkle in the morning sun.",
        "I'd like to welcome you to a new beginning.",
        "Experience the cozy, outdoor lifestyle.",
        "Earn money selling bugs and fish you catch...",
        "Or start up a farm of your own...",
        "Whatever you choose, you'll love it here...",
        "Welcome to Kindlewood Village.",
    };

    mCurrentLineIndex = 0;
    mCharIndex = 0;
    mDisplayedText = "";
}

void IntroScene::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm1);
    UpdateMusicStream(mGameState.bgm2);
    if (mCharIndex < mStoryLines[mCurrentLineIndex].length()) 
    {
        mTypewriterTimer += deltaTime;
        if (mTypewriterTimer > 0.04f)
        {
            mDisplayedText += mStoryLines[mCurrentLineIndex][mCharIndex];
            mCharIndex++;
            mTypewriterTimer = 0.0f;
            PlaySound(mGameState.sfx_1);
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
        mGameState.nextSceneID = 1;
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
    DrawRectangleLines(xPos, yPos, boxWidth, boxHeight, WHITE);

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