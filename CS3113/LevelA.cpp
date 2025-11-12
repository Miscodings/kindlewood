#include "LevelA.h"

LevelA::LevelA()                                      : Scene { {0.0f}, nullptr   } {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelA::~LevelA() {
   for (auto& entity : mGameState.entities) delete entity;
   mGameState.entities.clear();
   delete mGameState.player;
   delete mGameState.map;
   shutdown(); 
}

void LevelA::initialise()
{
   for (auto& entity : mGameState.entities) delete entity;
   mGameState.entities.clear();

   mGameState.nextSceneID = -1; 
   
   mGameState.bgm = LoadMusicStream("assets/game/music_level1.wav");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);

   mGameState.jumpSound = LoadSound("assets/game/step.ogg");
   mGameState.hurtSound = LoadSound("assets/game/hurt.ogg");
   mGameState.goalSound = LoadSound("assets/game/goal.ogg");

   mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT,   // map grid cols & rows
      (unsigned int *) mLevelData, // grid data
      "assets/game/grass.png",     // texture filepath
      TILE_DIMENSION,              // tile size
      9, 18,                       // texture cols & rows
      mOrigin                      // in-game origin
   );

   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {RIGHT,  { 0,  1, 2, 3 }},
      {LEFT, { 8, 9, 10, 11 }},
   };

   mGameState.player = new Entity(
      {mOrigin.x - 1050.0f, mOrigin.y - 10.0f}, // position
      {250.0f * 0.8f, 250.0f * 0.6f},           // scale
      "assets/game/sprites.png",                // texture file address
      ATLAS,                                    // single image or atlas?
      { 6, 8 },                                 // atlas dimensions
      playerAnimationAtlas,                    // actual atlas
      PLAYER                                    // entity type
   );
   
   float sizeRatio  = 48.0f / 64.0f;

   // std::map<Direction, std::vector<int>> boarAnimationAtlas = {
   //    {LEFT,  { 0, 1, 2, 3, 4, 5 }},
   //    {RIGHT, { 6, 7, 8, 9, 10, 11 }},
   // };

   // std::vector<Vector2> entityPositions = {
   //    {mOrigin.x - 425.0f, mOrigin.y - 10.0f}
   // };

   // for (auto& pos : entityPositions) {
   //    Entity* boar = new Entity(
   //       pos,
   //       {250.0f * 0.5f, 250.0f * 0.35f},
   //       "assets/game/boar_walk.png",
   //       ATLAS,
   //       {2, 6},
   //       boarAnimationAtlas,
   //       NPC
   //    );
   //    boar->setAIType(WANDERER);
   //    boar->setAIState(WALKING);
   //    boar->setColliderDimensions({
   //       boar->getScale().x / 4.5f,
   //       boar->getScale().y / 1.3f
   //    });

   //    mGameState.entities.push_back(boar);
   // }

   mGameState.player->setColliderDimensions({
      mGameState.player->getScale().x / 4.5f,
      mGameState.player->getScale().y / 1.5f
   });

   mGameState.camera = { 0 };                                    // zero initialize
   mGameState.camera.target = mGameState.player->getPosition(); // camera follows player
   mGameState.camera.offset = mOrigin;                           // camera offset to center of screen
   mGameState.camera.rotation = 0.0f;                            // no rotation
   mGameState.camera.zoom = 1.0f;                                // default zoom
}

void LevelA::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);
   
   mGameState.player->update(deltaTime, nullptr, nullptr, nullptr, 0);

   for (auto& entity : mGameState.entities) {
      entity->update(deltaTime, mGameState.player, nullptr, nullptr, 0);
   }

   Vector2 currentPlayerPosition = mGameState.player->getPosition();

   panCamera(&mGameState.camera, &currentPlayerPosition);
}

void LevelA::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));
   BeginMode2D(mGameState.camera);

   mGameState.map->render();
   mGameState.player->render();
   for (auto& entity : mGameState.entities) {
      entity->render();
   }

   EndMode2D();
}

void LevelA::shutdown()
{
   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.goalSound);
}