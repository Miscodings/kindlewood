#include "Level0.h"

Level0::Level0()                                      : Scene { {0.0f}, nullptr   } {}
Level0::Level0(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Level0::~Level0() { shutdown(); }

void Level0::initialise()
{
   mGameState.nextSceneID = -1; 

   mGameState.bgm = LoadMusicStream("assets/game/music_level1.wav");
   mBackgroundTexture = LoadTexture("assets/game/sky.png");
   PlayMusicStream(mGameState.bgm);

   mGameState.map = new Map(
      16, 16,
      16.0f,
      mOrigin   
   );
}

void Level0::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);

   if (IsKeyPressed(KEY_ENTER)) {
       mGameState.nextSceneID = 2;
   }
}

void Level0::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));
   float scaleX = (float)GetScreenWidth() / mBackgroundTexture.width;
   float scaleY = (float)GetScreenHeight() / mBackgroundTexture.height;

   DrawTexturePro(
      mBackgroundTexture,
      { 0, 0, (float)mBackgroundTexture.width, (float)mBackgroundTexture.height },
      { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
      { 0, 0 },
      0.0f,
      WHITE
   );
   
   mGameState.map->render();

   const char* titleText = "Kindlewood Village";
   int titleFontSize = 60;
   int titleTextWidth = MeasureText(titleText, titleFontSize);
   DrawText(titleText, (GetScreenWidth() - titleTextWidth) / 2, GetScreenHeight() / 4, titleFontSize, WHITE);

   const char* promptText = "Press ENTER to Start";
   int promptFontSize = 30;
   int promptTextWidth = MeasureText(promptText, promptFontSize);
   DrawText(promptText, (GetScreenWidth() - promptTextWidth) / 2, GetScreenHeight() / 2, promptFontSize, LIGHTGRAY);
}

void Level0::shutdown()
{
   delete mGameState.map;
   mGameState.map = nullptr;
   UnloadTexture(mBackgroundTexture);

   UnloadMusicStream(mGameState.bgm);
}