#include "LevelB.h"
#include "raymath.h"
#include <algorithm>

LevelB::LevelB()                                      : Scene { {0.0f}, nullptr   } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
   mGameState.nextSceneID = -1;
   mGameState.bgm1 = LoadMusicStream("assets/game/music_level3.mp3");
   SetMusicVolume(mGameState.bgm1, 0.7f);
   PlayMusicStream(mGameState.bgm1);

   mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT,
      TILE_DIMENSION,
      mOrigin
   );

   mGameState.map->addTileset("assets/game/Tileset_Interior.png", 1);

   mGameState.map->addLayer(
      std::vector<unsigned int>(std::begin(floor), std::end(floor)),
      false
   );
   mGameState.map->addLayer(
      std::vector<unsigned int>(std::begin(walls), std::end(walls)),
      true
   );

   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {DOWN,      { 1 }},
      {DOWN_WALK, { 0, 1, 2 }},
      {UP,        { 10 }},
      {UP_WALK,   { 9, 10, 11 }},
      {LEFT,      { 4 }},
      {LEFT_WALK, { 3, 4, 5 }},
      {RIGHT,     { 7 }},
      {RIGHT_WALK,{ 6, 7, 8 }},
   };

   mGameState.player = new Entity(
      { mOrigin.x, mOrigin.y + 75.0f},
      { 24.0f, 24.0f },            // Scale
      "assets/game/character.png", // Texture
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

   mGameState.player->loadPlayerData();

   mGameState.player->setColliderDimensions({
      mGameState.player->getScale().x / 4.0f,
      mGameState.player->getScale().y / 3.5f
   });

   mGameState.player->setSpeed(50.0f);

   mGameState.camera = { 0 };
   mGameState.camera.target = mGameState.player->getPosition();
   mGameState.camera.offset = mOrigin;
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.zoom = 4.0f;

   float mapX = mOrigin.x;
   float mapY = mOrigin.y;
   
   Entity* carrot_crate = new Entity(
      Vector2{mOrigin.x - 30.0f, mOrigin.y},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/carrot_crate.png",
      PROP
   );
   carrot_crate->setDialogue({"Buy|Carrot Seeds|15|20"}); 

   Entity* cauliflower_crate = new Entity(
      Vector2{mOrigin.x + 30.0f, mOrigin.y},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/cauliflower_crate.png",
      PROP
   );
   cauliflower_crate->setDialogue({"Buy|Cauliflower Seeds|15|15"});

   Entity* corn_crate = new Entity(
      Vector2{mOrigin.x - 60.0f, mOrigin.y},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/corn_crate.png",
      PROP
   );
   corn_crate->setDialogue({"Buy|Corn Seeds|10|13"}); 

   Entity* pepper_crate = new Entity(
      Vector2{mOrigin.x + 60.0f, mOrigin.y},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/pepper_crate.png",
      PROP
   );
   pepper_crate->setDialogue({"Buy|Pepper Seeds|15|16"});

   Entity* pineapple_crate = new Entity(
      Vector2{mOrigin.x - 30.0f, mOrigin.y + 30.0f},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/pineapple_crate.png",
      PROP
   );
   pineapple_crate->setDialogue({"Buy|Pineapple Seeds|15|17"});

   Entity* pumpkin_crate = new Entity(
      Vector2{mOrigin.x + 30.0f, mOrigin.y + 30.0f},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/pumpkin_crate.png",
      PROP
   );
   pumpkin_crate->setDialogue({"Buy|Pumpkin Seeds|15|19"});

   Entity* squash_crate = new Entity(
      Vector2{mOrigin.x - 60.0f, mOrigin.y + 30.0f},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/squash_crate.png",
      PROP
   );
   squash_crate->setDialogue({"Buy|Squash Seeds|15|18"});

   Entity* turnip_crate = new Entity(
      Vector2{mOrigin.x + 60.0f, mOrigin.y + 30.0f},
      Vector2{30.0f, 30.0f},
      "assets/game/Crates/turnip_crate.png",
      PROP
   );
   turnip_crate->setDialogue({"Buy|Turnip Seeds|15|14"});
   
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

   Entity* shopkeeper = new Entity(
      Vector2{mOrigin.x, mOrigin.y - 42.0f},
      Vector2{24.0f, 24.0f},
      "assets/game/npc_1.png",
      ATLAS, {4, 3},
      npcAnimationAtlas,                    
      NPC                                    
   );
   shopkeeper->setDialogue({
      "Shopkeeper|Hey there! I'll take those items off your hands. Come again!",
   });

   shopkeeper->setAIType(AI_IDLER);

   mGameState.entities.push_back(carrot_crate);
   mGameState.entities.push_back(cauliflower_crate);
   mGameState.entities.push_back(corn_crate);
   mGameState.entities.push_back(pepper_crate);
   mGameState.entities.push_back(pineapple_crate);
   mGameState.entities.push_back(pumpkin_crate);
   mGameState.entities.push_back(squash_crate);
   mGameState.entities.push_back(turnip_crate);
   mGameState.entities.push_back(shopkeeper);
}

void LevelB::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm1);

   mGameState.player->update(deltaTime, mGameState.player, mGameState.map, mGameState.entities);
   mGameState.camera.target = mGameState.player->getPosition();

   for (Entity* e : mGameState.entities) {
      if (e) e->update(deltaTime, mGameState.player, mGameState.map, mGameState.entities);
   }

   if (mGameState.player->getPosition().y > mGameState.map->getBottomBoundary() - 20)
   {
      mGameState.player->savePlayerData();
      Entity::setGlobalSpawnPosition({340.0f, 420.0f}); 
      mGameState.nextSceneID = 2;
   }

}

void LevelB::render()
{
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
   int money = mGameState.player->getMoney();
   
   std::string toolName = "Hands";
   ToolType t = mGameState.player->getTool();
   if (t == TOOL_NET) toolName = "Net";
   else if (t == TOOL_ROD) toolName = "Fishing Rod";
   else if (t == TOOL_HOE) toolName = "Hoe";
   else if (t == TOOL_WATERING_CAN) toolName = "Watering Can";

   DrawText(TextFormat("Money: $%d", money), 20, 20, 30, GOLD);
   DrawText(TextFormat("Tool: %s (TAB)", toolName.c_str()), 20, 60, 20, YELLOW);
   DrawText("SPACE: Interact | P: Sell", 20, 90, 10, LIGHTGRAY);

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
      return;
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
         DrawText(itemText, x + 10, y + 12, 10, WHITE);
      }
   }
}

void LevelB::shutdown()
{
   if (mGameState.map) {
      delete mGameState.map;
      mGameState.map = nullptr;
   }

   if (mGameState.player) {
      mGameState.player->savePlayerData();
   }

   if (mBackgroundTexture.id != 0) {
      UnloadTexture(mBackgroundTexture);
      mBackgroundTexture = { 0, 0 };
   }

   UnloadMusicStream(mGameState.bgm1);

   if (mGameState.player) {
      delete mGameState.player;
      mGameState.player = nullptr;
   }
   for (Entity* e : mGameState.entities) {
      if (e) delete e;
   }
   mGameState.entities.clear();
}
