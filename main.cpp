#include "CS3113/Level0.h"
#include "CS3113/LevelA.h"
#include "CS3113/IntroScene.h"
#include "CS3113/ShaderProgram.h"
#include "CS3113/Effects.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120;
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gLevels;

Level0 *gLevel0 = nullptr;
LevelA *gLevelA = nullptr;
IntroScene *gIntroScene = nullptr;

bool gIsTransitioning = false;

Effects *gEffects = nullptr;
ShaderProgram gShader;
Vector2 gLightPosition = { 0.0f, 0.0f };

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{   
    if (scene) {
        if (gCurrentScene) gCurrentScene->shutdown();   
        gCurrentScene = scene;
        gCurrentScene->initialise();
    }

}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kindlewood Sanctuary"); // Cute Title!
    InitAudioDevice();

    gShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");

    gIntroScene = new IntroScene(ORIGIN, "#000000ff");
    gLevel0 = new Level0(ORIGIN, "#76b6ff");
    gLevelA     = new LevelA(ORIGIN, "#76b6ff"); 

    gLevels.push_back(gIntroScene);
    gLevels.push_back(gLevel0);
    gLevels.push_back(gLevelA);

    switchToScene(gLevels[2]);

    gEffects = new Effects(ORIGIN, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    gEffects->start(FADEIN); // Fade into the intro

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (!gCurrentScene) return;
    if (gIsTransitioning) return; 

    if (gCurrentScene->isChatting()) 
    {
        if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            gCurrentScene->stopChat();
        }
        return;
    }

    if (gCurrentScene->getState().player) 
    {
        Entity* player = gCurrentScene->getState().player;
        player->resetMovement();

        // Movement
        if      (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  player->moveLeft();
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->moveRight();
        if      (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    player->moveUp();
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  player->moveDown();

        if (IsKeyPressed(KEY_TAB)) {
            player->cycleTool();
        }

        if (IsKeyPressed(KEY_SPACE)) {
            std::string actionResult = player->useTool(gCurrentScene->getState().entities);
            if (!actionResult.empty()) gCurrentScene->setChat(actionResult);
            else {
                std::string dialogue = player->interact(gCurrentScene->getState().entities);
                if (!dialogue.empty()) gCurrentScene->setChat(dialogue);
            }
        }
        if (IsKeyPressed(KEY_P)) player->sellItems();
    }
    if (IsKeyPressed(KEY_F) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    if (!gCurrentScene) return;

    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    gTimeAccumulator += deltaTime;

    while (gTimeAccumulator >= FIXED_TIMESTEP)
    {
        if (gCurrentScene) {
            // Only update the game logic if we ARE NOT transitioning out.
            // If we are fading to black, we usually want the player to stop moving.
            if (!gIsTransitioning) {
                gCurrentScene->update(FIXED_TIMESTEP);
            }
            
            if (gCurrentScene->getState().player) {
                gLightPosition = gCurrentScene->getState().player->getPosition();
            }
        }
        
        // Update effects (handles the alpha calculation)
        if (gEffects) gEffects->update(FIXED_TIMESTEP, nullptr);

        // --- TRANSITION LOGIC ---
        if (gCurrentScene && gCurrentScene->getState().nextSceneID != -1)
        {
            // 1. Start the Fade Out
            if (!gIsTransitioning) {
                gEffects->start(FADEOUT);
                gIsTransitioning = true;
            }
            // 2. Wait for Fade Out to complete
            // We check if the effect is finished (NONE) AND screen is black (Alpha >= 1.0)
            else if (gEffects->getCurrentEffect() == NONE && gEffects->getAlpha() >= 1.0f) {
                
                int id = gCurrentScene->getState().nextSceneID;
                
                // SWITCH SCENE NOW
                if (id >= 0 && id < gLevels.size()) {
                    switchToScene(gLevels[id]);
                }
                
                // 3. Start Fade In for the new scene
                gEffects->start(FADEIN);
                
                // Turn off the transition flag so the new scene can update
                gIsTransitioning = false; 
            }
        }

        gTimeAccumulator -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();
    bool applyShader = (gCurrentScene != gIntroScene && gCurrentScene != gLevel0);
    if (applyShader) {
        gShader.begin();
        gShader.setVector2("lightPosition", gLightPosition);
    }
    if (gCurrentScene) {
        gCurrentScene->render();
    }
    if (applyShader) {
        gShader.end();
    }
    if (gEffects) gEffects->render();
    EndDrawing();
}

void shutdown() 
{
    for (auto scene : gLevels) {
        delete scene;
    }
    gLevels.clear();

    delete gEffects;
    gEffects = nullptr;

    gShader.unload();

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}