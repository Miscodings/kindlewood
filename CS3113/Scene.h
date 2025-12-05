#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *player;
    Map *map;
    std::vector<Entity*> entities;

    Music bgm1;
    Music bgm2;

    Sound sfx_1;
    Sound sfx_2;
    Sound sfx_3;
    Sound sfx_4;
    Sound sfx_5;
    Sound sfx_6;
    Sound sfx_7;
    Sound sfx_8;
    Sound sfx_9;
    Sound sfx_10;

    Camera2D camera;

    int nextSceneID;
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    
    virtual void setChat(const std::string& text) {} 
    virtual bool isChatting() { return false; }
    virtual void stopChat() {}

    GameState& getState()                  { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif