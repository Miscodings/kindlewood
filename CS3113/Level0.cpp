#include "Level0.h"

Level0::Level0()                                      : Scene { {0.0f}, nullptr   } {}
Level0::Level0(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Level0::~Level0() { shutdown(); }

void Level0::initialise()
{
   mGameState.nextSceneID = -1; 

   mGameState.bgm1 = LoadMusicStream("assets/game/music_level3.mp3");
   mBackgroundTexture = LoadTexture("assets/game/sky.png");
   mLogo = LoadTexture("assets/game/logo.png");
   PlayMusicStream(mGameState.bgm1);

   mGameState.map = new Map(
      16, 16,
      16.0f,
      mOrigin   
   );
}

void Level0::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm1);

   if (IsKeyPressed(KEY_ENTER)) {
       mGameState.nextSceneID = 2;
   }
}

void Level0::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));
   float destW = GetScreenWidth() / 2.0f;
   float destH = GetScreenHeight() / 2.0f;

   DrawTexturePro(
      mBackgroundTexture,
      { 0, 0, (float)mBackgroundTexture.width, (float)mBackgroundTexture.height },
      { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
      { 0, 0 },
      0.0f,
      WHITE
   );

   DrawTexturePro(
      mLogo,
      { 0, 0, (float)mLogo.width, (float)mLogo.height },
      { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f - 150.0f, destW, destH },
      { destW / 2.0f, destH / 2.0f },
      0.0f,
      WHITE
   );

   mGameState.map->render();

   const char* promptText = "Press ENTER to Start";
   int promptFontSize = 30;
   int promptTextWidth = MeasureText(promptText, promptFontSize);
   DrawText(promptText, (GetScreenWidth() - promptTextWidth) / 2, GetScreenHeight() / 2 + 100.0f, promptFontSize, LIGHTGRAY);
}

void Level0::shutdown()
{
   delete mGameState.map;
   mGameState.map = nullptr;
   UnloadTexture(mBackgroundTexture);

   UnloadMusicStream(mGameState.bgm1);
}