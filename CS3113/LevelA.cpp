#include "LevelA.h"
#include <algorithm>

LevelA::LevelA()                              : Scene { {0.0f}, nullptr   } {}
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
      LEVEL_WIDTH, LEVEL_HEIGHT,   
      TILE_DIMENSION,              
      mOrigin                      
   );

   int groundStart  = 1;
   int waterStart   = 1000;
   int roadStart    = 2000;
   int flowerStart  = 3000;
   int flowerWhiteStart = 3500;

   mGameState.map->addTileset("assets/game/Tileset_Ground.png",            groundStart); 
   mGameState.map->addTileset("assets/game/Tileset_Water.png",             waterStart);
   mGameState.map->addTileset("assets/game/Tilesets_Road.png",             roadStart);
   mGameState.map->addTileset("assets/game/Animation_Flowers_Red.png",     flowerStart);
   mGameState.map->addTileset("assets/game/Animation_Flowers_White.png",   flowerWhiteStart);

   for(unsigned int &id : mDataGround) {
      if (id != 0) {
         id += 1;
      }
   }

   for(unsigned int &id : mDataWater) {
      if (id == 173) { id = 0; }
      else if (id != 0) {
         id += (waterStart - 1);
         id += 1;
      }
   }

   for(unsigned int &id : mDataRoad) {
      if (id == 47) { id = 0; }
      else if (id != 0) {
         id += (roadStart - 1);
         id += 1;
      }
   }

   for(unsigned int &id : mDataFlowers) {
      if (id != 0) {
         id += (flowerStart - 1);
         id += 1;
      }
   }
   
   mGameState.map->addLayer(mDataGround, false); 
   mGameState.map->addLayer(mDataWater, true);
   mGameState.map->addLayer(mDataRoad, false);
   mGameState.map->addLayer(mDataFlowers, false);

   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {DOWN,  { 1 }},
      {DOWN_WALK, { 0, 1, 2 }},
      {UP,  { 10 }},
      {UP_WALK, { 9, 10, 11 }},
      {LEFT,  { 4 }},
      {LEFT_WALK, { 3, 4, 5 }},
      {RIGHT,  { 7 }},
      {RIGHT_WALK, { 6, 7, 8 }},
   };

   mGameState.player = new Entity(
      {mOrigin.x + 30.0f, mOrigin.y + 30.0f},   // Spawn position
      {24.0f, 24.0f},                           // Scale (32px is 2 tiles wide)
      "assets/game/character.png",              // Texture
      ATLAS,                                    
      { 4, 3 },                                 
      playerAnimationAtlas,                    
      PLAYER                                    
   );
   
   mGameState.player->setColliderDimensions({
      mGameState.player->getScale().x / 4.0f,
      mGameState.player->getScale().y / 3.5f
   });

   mGameState.player->setPosition({268, 105});
   mGameState.player->setSpeed(50.0f);

   buildForest();
   buildVillage();
   spawnNPCs();

   mGameState.camera = { 0 };                                     
   mGameState.camera.target = mGameState.player->getPosition();   
   mGameState.camera.offset = mOrigin;                            
   mGameState.camera.rotation = 0.0f;                             
   mGameState.camera.zoom = 4.0f;                                 
}

void LevelA::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);
   
   int bugCount = 0;
   for (Entity* e : mGameState.entities) {
      if (e->getEntityType() == BUG && e->isActive()) {
         bugCount++;
      }
   }

   if (bugCount < 4) {
      mBugRespawnTimer += deltaTime;
      if (mBugRespawnTimer > 3.0f) {
         spawnBug();
         mBugRespawnTimer = 0.0f;
      }
   }

   // Pause updates if chatting
   // Input for closing chat is handled in main.cpp
   if (mIsChatting) return;

   mGameState.player->update(deltaTime, mGameState.player, mGameState.map, mGameState.entities);

   for (auto& entity : mGameState.entities) {
      entity->update(deltaTime, mGameState.player, mGameState.map, mGameState.entities);
   }

   Vector2 pos = mGameState.player->getPosition();
   Vector2 col = mGameState.player->getColliderDimensions();

   float mapLeft   = mGameState.map->getLeftBoundary();
   float mapRight  = mGameState.map->getRightBoundary();
   float mapTop    = mGameState.map->getTopBoundary();
   float mapBottom = mGameState.map->getBottomBoundary();

   if (pos.x - (col.x / 2.0f) < mapLeft)   pos.x = mapLeft + (col.x / 2.0f);
   if (pos.x + (col.x / 2.0f) > mapRight)  pos.x = mapRight - (col.x / 2.0f);

   if (pos.y - (col.y / 2.0f) < mapTop)    pos.y = mapTop + (col.y / 2.0f);
   if (pos.y + (col.y / 2.0f) > mapBottom) pos.y = mapBottom - (col.y / 2.0f);

   mGameState.player->setPosition(pos);

   Vector2 currentPlayerPosition = mGameState.player->getPosition();
   float screenWidth = GetScreenWidth() / mGameState.camera.zoom;
   float screenHeight = GetScreenHeight() / mGameState.camera.zoom;

   float minX = mapLeft + screenWidth / 2.0f;
   float maxX = mapRight - screenWidth / 2.0f;
   float minY = mapTop + screenHeight / 2.0f;
   float maxY = mapBottom - screenHeight / 2.0f;

   // Prevent camera from inverting if screen is bigger than map
   if (minX > maxX) minX = maxX = (mapLeft + mapRight) / 2.0f;
   if (minY > maxY) minY = maxY = (mapTop + mapBottom) / 2.0f;

   mGameState.camera.target.x = fmaxf(minX, fminf(currentPlayerPosition.x, maxX));
   mGameState.camera.target.y = fmaxf(minY, fminf(currentPlayerPosition.y, maxY));
}

void LevelA::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));
   BeginMode2D(mGameState.camera);

   if (mGameState.map) mGameState.map->render();

   std::vector<Entity*> renderQueue;
   if (mGameState.player) renderQueue.push_back(mGameState.player);
   for (auto& entity : mGameState.entities) {
         renderQueue.push_back(entity);
   }

   std::sort(renderQueue.begin(), renderQueue.end(), [](Entity* a, Entity* b) {
         return a->getPosition().y < b->getPosition().y;
   });

   for (Entity* e : renderQueue) {
         e->render();
         // e->displayCollider();
   }

   EndMode2D();
   EndShaderMode();

   int money = mGameState.player->getMoney();
   int bagSize = mGameState.player->getInventorySize();
   
   std::string toolName = "Hands";
   ToolType t = mGameState.player->getTool();
   if (t == TOOL_NET) toolName = "Net";
   else if (t == TOOL_ROD) toolName = "Rod";
   else if (t == TOOL_AXE) toolName = "Axe";

   DrawText(TextFormat("Money: $%d", money), 20, 20, 30, GOLD);
   DrawText(TextFormat("Bag: %d Items", bagSize), 20, 60, 20, WHITE);
   DrawText(TextFormat("Tool: %s (TAB)", toolName.c_str()), 20, 90, 20, YELLOW);
   DrawText("SPACE: Act | P: Sell", 20, 120, 10, LIGHTGRAY);

   if (mIsChatting)
   {
      DrawRectangle(50, 450, 900, 130, Fade(BLACK, 0.8f));
      DrawRectangleLines(50, 450, 900, 130, WHITE);

      std::string speakerName = "";
      std::string messageContent = mCurrentChatText;
      
      size_t splitPosition = mCurrentChatText.find('|');

      if (splitPosition != std::string::npos) {
         speakerName    = mCurrentChatText.substr(0, splitPosition);
         messageContent = mCurrentChatText.substr(splitPosition + 1);
      }

      if (!speakerName.empty()) {
         DrawText(speakerName.c_str(), 70, 460, 20, YELLOW);
      }

      DrawText(messageContent.c_str(), 70, 490, 24, WHITE);

      DrawText(">>", 900, 550, 20, (int)GetTime() % 2 == 0 ? WHITE : GRAY);
   }
}

void LevelA::shutdown()
{
   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.goalSound);
}

void LevelA::buildForest()
{
   float mapX = mGameState.map->getLeftBoundary();
   float mapY = mGameState.map->getTopBoundary();

   std::vector<Vector2> treePositions = {
      {50, 35}, {90, 25}, {130, 30}, {170, 25}, {210, 31}, {250, 28}, {290, 30}, {330, 37}, {370, 23}, {410, 40},
   };

   std::vector<Vector2> rockPositions = {
      {300, 300}, {450, 200}, {50, 500}
   };

   for (Vector2 pos : treePositions) {
      int variant = GetRandomValue(1, 4); 
      const char* path = TextFormat("assets/game/tree_%d.png", variant);

      Entity* tree = new Entity(
         {mapX + pos.x, mapY + pos.y},    // Spawn position
         {32.0f, 48.0f},                  // Scale
         path,                            // Texture
         PROP
      );
      tree->setColliderDimensions({10.0f, 20.0f}); 
      mGameState.entities.push_back(tree);
   }

   for (Vector2 pos : rockPositions) {
      int variant = GetRandomValue(1, 5);
      const char* path = TextFormat("assets/game/rock_%d.png", variant);

      Entity* rock = new Entity(
         {mapX + pos.x, mapY + pos.y},
         {16.0f, 16.0f},
         path,
         PROP
      );
      rock->setColliderDimensions({12.0f, 12.0f});
      mGameState.entities.push_back(rock);
   }

   std::map<Direction, std::vector<int>> bugAnims = {
       {LEFT,       {18, 19, 20}},
       {RIGHT,      {6, 7, 8}},
       {UP,         {0, 1, 2}},
       {DOWN,       {12, 13, 14}},
       {LEFT_WALK,  {18, 19, 20}},
       {RIGHT_WALK, {6, 7, 8}},
       {UP_WALK,    {0, 1, 2}},
       {DOWN_WALK,  {12, 13, 14}}
   };

   for(int i = 0; i < 4; i++) {
      spawnBug();
   }

   Entity* fish = new Entity(
      {600, 360}, 
      {24.0f, 24.0f}, 
      "assets/game/fish_shadow.png",
      FISH
   );

   fish->setColliderDimensions({20,20});
   fish->setAIType(WANDERER); 
   fish->setAIState(WALKING);
   mGameState.entities.push_back(fish);
}

void LevelA::buildVillage()
{
   float mapX = mGameState.map->getLeftBoundary();
   float mapY = mGameState.map->getTopBoundary();

   std::vector<Vector2> housePositions = {
      {90, 95},
      {400, 350},
      {550, 150}
   };

   for (Vector2 pos : housePositions) {
      Entity* house = new Entity(
         {mapX + pos.x, mapY + pos.y},
         {80.0f, 80.0f},
         "assets/game/house_1.png",
         PROP
      );
      house->setColliderDimensions({60.0f, 40.0f}); 
      mGameState.entities.push_back(house);
   }

   Entity* store = new Entity(
      {mapX + 140, mapY + 405},
      {80.0f, 80.0f},
      "assets/game/store.png",
      PROP
   );
   store->setColliderDimensions({60.0f, 40.0f}); 
   mGameState.entities.push_back(store);
}

void LevelA::spawnNPCs()
{
   float mapX = mGameState.map->getLeftBoundary();
   float mapY = mGameState.map->getTopBoundary();

   std::map<Direction, std::vector<int>> npcAnim = {
      {LEFT, { 0, 1, 2, 3 }},
      {RIGHT, { 6, 7, 8, 9 }}
   };

   Entity* bob = new Entity(
      {mapX + 300, mapY + 300},
      {50.0f, 32.0f},
      "assets/game/boar_walk.png",
      ATLAS, {2, 6}, npcAnim,
      NPC
   );
   
   bob->setSpeed(20.0f);
   bob->setColliderDimensions({30.0f, 16.0f}); 
   bob->setAIType(WANDERER);
   bob->setAIState(WALKING);
   bob->setDialogue("Bob|Welcome to Kindlewood! Watch out for bees.");
   
   mGameState.entities.push_back(bob);
}

void LevelA::spawnBug()
{
   float mapWidth = mGameState.map->getRightBoundary();
   float mapHeight = mGameState.map->getBottomBoundary();

   float randX = (float)GetRandomValue(100, (int)mapWidth - 100);
   float randY = (float)GetRandomValue(100, (int)mapHeight - 100);

   std::map<Direction, std::vector<int>> bugAnims = {
      {LEFT,       {0, 1, 2}}, {RIGHT,      {0, 1, 2}},
      {UP,         {0, 1, 2}}, {DOWN,       {0, 1, 2}},
      {LEFT_WALK,  {0, 1, 2}}, {RIGHT_WALK, {0, 1, 2}},
      {UP_WALK,    {0, 1, 2}}, {DOWN_WALK,  {0, 1, 2}}
   };

   Entity* bug = new Entity(
      {randX, randY}, 
      {16.0f, 16.0f}, 
      "assets/game/butterfly.png", 
      ATLAS, {8, 3}, bugAnims, BUG
   );

   bug->setFrameSpeed(10);
   bug->setColliderDimensions({10,10});
   bug->setAIType(WANDERER);
   bug->setSpeed(40);
   
   mGameState.entities.push_back(bug);
}