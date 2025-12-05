#include "LevelA.h"
#include <algorithm>
#include <vector>
#include "raymath.h" 

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
   // Clear old entities
   for (auto& entity : mGameState.entities) delete entity;
   mGameState.entities.clear();

   mGameState.nextSceneID = -1; 
   
   mGameState.bgm = LoadMusicStream("assets/game/music_level1.wav");
   SetMusicVolume(mGameState.bgm, 0.02f);
//   PlayMusicStream(mGameState.bgm);

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

   // --- FIX: Create Local Copies so we don't corrupt the master data ---
   std::vector<unsigned int> groundMap = mDataGround;
   std::vector<unsigned int> waterMap  = mDataWater;
   std::vector<unsigned int> roadMap   = mDataRoad;
   std::vector<unsigned int> flowerMap = mDataFlowers;

   // Adjust IDs on the LOCAL copies
   for(unsigned int &id : groundMap) {
      if (id != 0) id += 1;
   }

   for(unsigned int &id : waterMap) {
      if (id == 173) { id = 0; }
      else if (id != 0) {
         id += (waterStart - 1);
         id += 1;
      }
   }

   for(unsigned int &id : roadMap) {
      if (id == 47) { id = 0; }
      else if (id != 0) {
         id += (roadStart - 1);
         id += 1;
      }
   }

   for(unsigned int &id : flowerMap) {
      if (id != 0) {
         id += (flowerStart - 1);
         id += 1;
      }
   }
   
   mGameState.map->addLayer(groundMap, false); 
   mGameState.map->addLayer(waterMap, true);
   mGameState.map->addLayer(roadMap, false);
   mGameState.map->addLayer(flowerMap, false);

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

   // --- POSITIONING LOGIC ---
   Vector2 spawnPos = {268, 105}; // Default start
   
   // Check if we returned from the house (Global Spawn override)
   Vector2 savedPos = Entity::getGlobalSpawnPosition();
   if (savedPos.x != -1.0f) {
       spawnPos = savedPos;
       Entity::setGlobalSpawnPosition({-1.0f, -1.0f}); // Reset after use
   }

   mGameState.player = new Entity(
      spawnPos,
      {24.0f, 24.0f},
      "assets/game/character.png",
      ATLAS,
      { 4, 3 },
      playerAnimationAtlas,
      PLAYER
   );
   
   mGameState.player->loadToolTextures(
      "assets/game/net.png", 
      "assets/game/rod.png",
      "assets/game/bucket.png",
      "assets/game/hoe.png"
   );
   
   mGameState.player->setColliderDimensions({
      mGameState.player->getScale().x / 4.0f,
      mGameState.player->getScale().y / 3.5f
   });

   mGameState.player->setSpeed(50.0f);

   mGameState.player->loadPlayerData(); 

   buildVillage();
   buildForest(); 
   spawnNPCs();

   mDayCount = Entity::sGlobalDayCount;
   mTimeOfDay = Entity::sGlobalTimeOfDay;
   for (const auto& cropData : Entity::sGlobalCrops) {
      Entity* crop = new Entity(
         cropData.position,
         {16.0f, 16.0f},
         "assets/game/Tileset_Farming.png",
         CROP
      );
      crop->setColliderDimensions({14, 14});
      
      // Restore State
      crop->setCropType(cropData.type);
      crop->setWatered(cropData.isWatered);
      crop->setGrowthStage(cropData.growthStage); 
      mGameState.entities.push_back(crop);
   }
   mGameState.camera = { 0 };                                     
   mGameState.camera.target = mGameState.player->getPosition();   
   mGameState.camera.offset = mOrigin;                            
   mGameState.camera.rotation = 0.0f;                             
   mGameState.camera.zoom = 4.0f;                                 
}

void LevelA::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);

   // --- BUG RESPAWN LOGIC ---
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

   if (mIsChatting) {
      if (mCurrentChatText == "ACTION_ENTER_HOUSE") {
         stopChat();
         mGameState.nextSceneID = 3;
      }
      return;
   }

   mTimeOfDay += deltaTime / 120.0f; 

   if (mTimeOfDay >= 1.0f) {
      mTimeOfDay = 0.0f;
      mDayCount++;

      for (Entity* e : mGameState.entities) {
         if (e->getEntityType() == CROP) {
               e->growCrop();
         }
         if (e->getEntityType() == BUG && !e->isActive()) {
               e->activate();
         }
      }
      setChat("Cock-a-doodle-doo!|It's a new day!");
   }

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

   if (minX > maxX) minX = maxX = (mapLeft + mapRight) / 2.0f;
   if (minY > maxY) minY = maxY = (mapTop + mapBottom) / 2.0f;

   mGameState.camera.target.x = fmaxf(minX, fminf(currentPlayerPosition.x, maxX));
   mGameState.camera.target.y = fmaxf(minY, fminf(currentPlayerPosition.y, maxY));
}

void LevelA::render()
{
    // --- WORLD RENDER ---
    ClearBackground(ColorFromHex(mBGColourHexCode));
    BeginMode2D(mGameState.camera);

    if (mGameState.map) mGameState.map->render();

    // Sort entities by Y position for depth
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
    }

    EndMode2D();
    EndShaderMode();
   
    float alpha = 0.0f;
    if (mTimeOfDay > 0.6f) {
        alpha = (mTimeOfDay - 0.6f) * 2.5f; 
        if (alpha > 0.75f) alpha = 0.75f; 
    }
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(mNightColor, alpha));

    int hour = (int)(mTimeOfDay * 24.0f) + 6; // Start at 6:00 AM
    if (hour >= 24) hour -= 24;
    DrawText(TextFormat("Day %d | %02d:00", mDayCount, hour), GetScreenWidth() - 160, 20, 20, WHITE);

    int money = mGameState.player->getMoney();
   
    std::string toolName = "Hands";
    ToolType t = mGameState.player->getTool();
    if (t == TOOL_NET) toolName = "Net";
    else if (t == TOOL_ROD) toolName = "Fishing Rod";
    else if (t == TOOL_HOE) toolName = "Hoe";
    else if (t == TOOL_WATERING_CAN) toolName = "Watering Can";

    DrawText(TextFormat("Money: $%d", money), 20, 20, 30, GOLD);
    DrawText(TextFormat("Tool: %s (TAB)", toolName.c_str()), 20, 60, 20, YELLOW);
    DrawText("SPACE: Use Tool | E: Interact | P: Sell", 20, 90, 10, LIGHTGRAY);

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
        return; // Don't draw inventory while chatting
    }

    const int SLOT_SIZE     = 40;
    const int SLOT_PADDING  = 6;
    const int NUM_SLOTS     = 9;
    const int BAR_BOTTOM_MARGIN = 50;

    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int totalBarWidth = (NUM_SLOTS * SLOT_SIZE) + ((NUM_SLOTS - 1) * SLOT_PADDING);
    int startX = (screenWidth - totalBarWidth) / 2;
    int startY = screenHeight - BAR_BOTTOM_MARGIN - SLOT_SIZE;

    const std::vector<Item>& inventory = mGameState.player->getInventory();

    for (int i = 0; i < NUM_SLOTS; i++)
    {
        int x = startX + i * (SLOT_SIZE + SLOT_PADDING);
        int y = startY;

        DrawRectangle(x, y, SLOT_SIZE, SLOT_SIZE, Fade(BLACK, 0.6f));
        DrawRectangleLines(x, y, SLOT_SIZE, SLOT_SIZE, LIGHTGRAY);
        DrawText(TextFormat("%d", i + 1), x + 2, y + 2, 10, GRAY);

        if (i < (int)inventory.size())
        {
            ItemType type = inventory[i].type;
            
            Color itemColor = WHITE;
            const char* itemText = "?";

            switch (type) {
                case ITEM_APPLE:      itemColor = RED;         itemText = "Ap"; break;
                case ITEM_BASS:       itemColor = BLUE;        itemText = "Fi"; break;
                case ITEM_BUTTERFLY:  itemColor = GREEN;       itemText = "Bg"; break;
                
                case ITEM_CORN:           itemColor = YELLOW;      itemText = "Co"; break;
                case ITEM_TURNIP:         itemColor = PURPLE;      itemText = "Tu"; break;
                case ITEM_CAULIFLOWER:    itemColor = GREEN;       itemText = "Cau"; break;
                case ITEM_PEPPER:         itemColor = RED;         itemText = "Pe"; break;
                case ITEM_PINEAPPLE:      itemColor = YELLOW;      itemText = "Pi"; break;
                case ITEM_SQUASH:         itemColor = GREEN;       itemText = "Sq"; break;
                case ITEM_PUMPKIN:        itemColor = ORANGE;      itemText = "Pu"; break;
                case ITEM_CARROT:         itemColor = ORANGE;      itemText = "Ca"; break;
                
                case SEEDS_CORN:          itemColor = DARKGREEN;   itemText = "S-C"; break;
                case SEEDS_TURNIP:        itemColor = DARKGREEN;   itemText = "S-T"; break;
                case SEEDS_CAULIFLOWER:   itemColor = DARKGREEN;   itemText = "S-Cau"; break;
                case SEEDS_PEPPER:        itemColor = DARKGREEN;   itemText = "S-P"; break;
                case SEEDS_PINEAPPLE:     itemColor = DARKGREEN;   itemText = "S-Pi"; break;
                case SEEDS_SQUASH:        itemColor = DARKGREEN;   itemText = "S-Sq"; break;
                case SEEDS_PUMPKIN:       itemColor = DARKGREEN;   itemText = "S-Pu"; break;
                case SEEDS_CARROT:        itemColor = DARKGREEN;   itemText = "S-Ca"; break;

                default: break;
            }
            DrawRectangle(x + 8, y + 8, 24, 24, itemColor);
            DrawText(itemText, x + 10, y + 12, 10, WHITE); // Centered Text
        }
    }
}

void LevelA::shutdown()
{
   if (mGameState.player) {
       mGameState.player->savePlayerData();
   }
   Entity::sGlobalDayCount = mDayCount;
   Entity::sGlobalTimeOfDay = mTimeOfDay;
   Entity::sGlobalCrops.clear();
   for (Entity* e : mGameState.entities) {
       if (e->getEntityType() == CROP) {
           Entity::sGlobalCrops.push_back({
               e->getPosition(),
               e->getCropType(),
               e->getGrowthStage(),
               e->isWatered()
           });
       }
   }

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.goalSound);
}

void LevelA::buildForest()
{
   std::vector<Vector2> treePositions = {
      {30,40}, {70,30}, {150,20}, {200,45}, {260,30}, 
      {320,15}, {380,25}, {440,40}, {500,20}, {580,35}, {620, 25},
      {20,150}, {50,200}, {25,260}, {60,320}, {220,150}, {250,180}, 
      {100,350}, {150,280}, {380,240}, {480,120}, {500,80}, {590,250}, {630,300}, 
      {550, 150}, {600, 120}, {630, 180}, {580, 320}, {610, 280},
      {560, 400}, {620, 380}, {590, 430}, {50, 500}, {80,540}, {120, 580}, 
      {170,530}, {240,540}, {280, 560}, {320,450}, {360,530}, {450,520}, 
      {680, 530}, {520,400}, {550,450}, {600,480}, {680, 530}, {700, 550},
      {550, 580}, {600, 600}
   };

   for (Vector2 pos : treePositions) {
       int variant = ((int)pos.x + (int)pos.y) % 3 + 1; 
       const char* path = TextFormat("assets/game/tree_%d.png", variant);
       
       Entity* tree = new Entity(
            pos,
            {32.0f, 48.0f},
            path,
            PROP
       );
       
       tree->setColliderDimensions({16.0f, 24.0f}); 
       mGameState.entities.push_back(tree);
   }

   std::vector<Vector2> rockPositions = { 
       {300, 300}, {450, 200}, {50, 500}, 
       {180, 120}, {580, 350}, {350, 550} 
   };

   for (Vector2 pos : rockPositions) {
      Entity* rock = new Entity(pos, {12.0f, 12.0f}, "assets/game/rock_1.png", PROP);
      rock->setColliderDimensions({12.0f, 12.0f});
      mGameState.entities.push_back(rock);
   }

   for(int i = 0; i < 4; i++) spawnBug();
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
   store->setDialogue({"ACTION_ENTER_HOUSE"}); 
   mGameState.entities.push_back(store);
}

void LevelA::spawnNPCs()
{
   float mapX = mGameState.map->getLeftBoundary();
   float mapY = mGameState.map->getTopBoundary();

   std::map<Direction, std::vector<int>> npcAnimationAtlas = {
      {DOWN,  { 1 }},
      {DOWN_WALK, { 0, 1, 2 }},
      {UP,  { 10 }},
      {UP_WALK, { 9, 10, 11 }},
      {LEFT,  { 4 }},
      {LEFT_WALK, { 3, 4, 5 }},
      {RIGHT,  { 7 }},
      {RIGHT_WALK, { 6, 7, 8 }},
   };

   Entity* sally = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_1.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* mark = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_2.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);   
   Entity* sam = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_3.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* alex = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_4.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* taylor = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_5.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* julia = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_6.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* jasmine = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_7.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   Entity* jessica = new Entity(
      {mapX + 600, mapY + 300}, {24.0f, 24.0f},
      "assets/game/npc_8.png",
      ATLAS, {4, 3}, npcAnimationAtlas, NPC);
   
   mark->setPosition({150, 200});
   mark->setDialogue({"Mark|Lovely day, isn't it?"});
   mark->setAIType(AI_WANDERER);
   sally->setPosition({400, 105});
   sally->setDialogue({"Sally|Hey! If you're looking for the store, it's near the south river!"});
   sally->setAIType(AI_IDLER);
   sam->setPosition({250, 400});
   sam->setDialogue({"Sam|I love fishing by the river."});
   sam->setAIType(AI_WANDERER);
   alex->setPosition({500, 350});
   alex->setDialogue({"Alex|I. LOVE. EXERCISE."});
   alex->setAIType(AI_CIRCLE);
   taylor->setPosition({120, 150});
   taylor->setDialogue({"Taylor|The crops are growing well this season."});
   taylor->setAIType(AI_WANDERER);
   julia->setPosition({300, 250});
   julia->setDialogue({"Julia|Press TAB to switch tool!"});
   julia->setAIType(AI_WANDERER);
   jasmine->setPosition({550, 200});
   jasmine->setDialogue({"Jasmine|The flowers here are so beautiful!"});
   jasmine->setAIType(AI_WANDERER);
   jessica->setPosition({200, 500});
   jessica->setDialogue({"Jessica|Have you seen any butterflies around?"});
   jessica->setAIType(AI_WANDERER);
   
   std::vector<Entity*> npcList;
   npcList.push_back(sally);
   npcList.push_back(mark);
   npcList.push_back(sam);
   npcList.push_back(alex);
   npcList.push_back(taylor);
   npcList.push_back(julia);
   npcList.push_back(jasmine);
   npcList.push_back(jessica);

   for (Entity* npc : npcList) {
      npc->setSpeed(20.0f);
      npc->setColliderDimensions({
         npc->getScale().x / 4.0f,
         npc->getScale().y / 3.5f
      });
      mGameState.entities.push_back(npc);
   }
}

void LevelA::spawnBug()
{
   float mapWidth = mGameState.map->getRightBoundary();
   float mapHeight = mGameState.map->getBottomBoundary();

   float randX = (float)GetRandomValue(100, (int)mapWidth - 100);
   float randY = (float)GetRandomValue(100, (int)mapHeight - 100);

   int prob = GetRandomValue(0, 5);
   std::string bugTexture;

   if (prob == 0) {
      bugTexture = "assets/game/Butterflies/butterfly_blue.png";
   } else if (prob == 1) {
      bugTexture = "assets/game/Butterflies/butterfly_gray.png";
   } else if (prob == 2) {
      bugTexture = "assets/game/Butterflies/butterfly_pink.png";
   } else if (prob == 3) {
      bugTexture = "assets/game/Butterflies/butterfly_red.png";
   } else if (prob == 4) {
      bugTexture = "assets/game/Butterflies/butterfly_white.png";
   } else {
      bugTexture = "assets/game/Butterflies/butterfly_yellow.png";
   }

   std::map<Direction, std::vector<int>> bugAnims = {
      {LEFT,       {0, 1, 2}}, {RIGHT,      {0, 1, 2}},
      {UP,         {0, 1, 2}}, {DOWN,       {0, 1, 2}},
      {LEFT_WALK,  {0, 1, 2}}, {RIGHT_WALK, {0, 1, 2}},
      {UP_WALK,    {0, 1, 2}}, {DOWN_WALK,  {0, 1, 2}}
   };

   Entity* bug = new Entity(
      {randX, randY}, 
      {8.0f, 8.0f}, 
      bugTexture.c_str(), 
      ATLAS, {1, 5}, bugAnims, BUG
   );

   bug->setFrameSpeed(10);
   bug->setColliderDimensions({10,10});
   bug->setAIType(AI_WANDERER);
   bug->setSpeed(40);
   
   mGameState.entities.push_back(bug);
}
