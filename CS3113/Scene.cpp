#include "Scene.h"

Scene::Scene() : mOrigin{ 0.0f, 0.0f }
{
    mGameState.player      = nullptr;
    mGameState.map          = nullptr;
    mGameState.entities      = {};
    mGameState.nextSceneID  = -1;
    mGameState.camera       = { 0 };
}

Scene::Scene(Vector2 origin, const char *bgHexCode) : mOrigin{origin}, mBGColourHexCode{bgHexCode} 
{
    mGameState.player      = nullptr;
    mGameState.map          = nullptr;
    mGameState.entities      = {};
    mGameState.nextSceneID  = -1;
    mGameState.camera       = { 0 };
}