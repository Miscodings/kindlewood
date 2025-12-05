#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, RIGHT, UP, DOWN, LEFT_WALK, RIGHT_WALK, UP_WALK, DOWN_WALK        };
enum EntityStatus { ACTIVE, INACTIVE                                                        };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, PROP, COLLECTIBLE, BUG, FISH, MISC, CROP  };
enum AIType       { AI_WANDERER, AI_IDLER, AI_CIRCLE                                        };

enum ToolType { TOOL_NONE, TOOL_NET, TOOL_ROD, TOOL_HOE, TOOL_WATERING_CAN                  };

enum ItemType { ITEM_NONE, ITEM_APPLE, ITEM_GEM, ITEM_BUTTERFLY, ITEM_BASS,
                ITEM_CORN, ITEM_TURNIP, ITEM_CAULIFLOWER, ITEM_PEPPER, ITEM_PINEAPPLE, ITEM_SQUASH, ITEM_PUMPKIN, ITEM_CARROT,
                SEEDS_CORN, SEEDS_TURNIP, SEEDS_CAULIFLOWER, SEEDS_PEPPER, SEEDS_PINEAPPLE, SEEDS_SQUASH, SEEDS_PUMPKIN, SEEDS_CARROT };

enum CropType { CROP_NONE, CROP_CORN, CROP_TURNIP, CROP_CAULIFLOWER, CROP_PEPPER, CROP_PINEAPPLE, CROP_SQUASH, CROP_PUMPKIN, CROP_CARROT };

struct Item {
    ItemType type;
    int value;
};

class Entity
{
private:

    int mMoney = 0;
    std::vector<Item> mInventory;
    std::string mDialogueText = "...";
    ToolType mEquippedTool = TOOL_NONE;

    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;
    
    Texture2D mTextureNet;
    Texture2D mTextureRod;
    Texture2D mTextureWateringCan;
    Texture2D mTextureHoe;

    TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;
    
    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    int mSpeed;
    float mAngle;
    float mShakeTimer;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType mAIType;
    float mAITimer = 0.0f;
    float mAIDecisionTime = 1.5f;
    Vector2 mAIDirection = {0, 0};
    float mCircleAngle = 0.0f;
    Vector2 mAnchorPosition = {0.0f, 0.0f};

    bool isColliding(Entity *other) const;

    void checkCollisionY(const std::vector<Entity*>& collidableEntities);
    void checkCollisionX(const std::vector<Entity*>& collidableEntities);
    void checkCollisionX(Map *map);
    void checkCollisionY(Map *map);
    void updateCircle(); 
    
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void AIUpdate(Map* map);
    void updateWanderer(Map* map);
    void updateIdler();

    void drawTool();

    bool mIsFishing;
    bool mFishOnHook;
    float mFishingTimer;
    float mHookWindowTimer;

    CropType mCropType;
    int mGrowthStage;
    bool mIsWatered;

    std::vector<std::string> mDialogueLines;
    int mDialogueIndex = 0;

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 100;
    static constexpr int   DEFAULT_FRAME_SPEED   = 14;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        EntityType entityType);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, 
        EntityType entityType);
    ~Entity();

    void update(float deltaTime, Entity *player, Map *map, 
        const std::vector<Entity*>& collidableEntities); 

        
    void updateAnimationOnly(float deltaTime) { animate(deltaTime); }
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    void activate()   { mEntityStatus  = ACTIVE;   }
    void deactivate() { mEntityStatus  = INACTIVE; }
    void displayCollider();

    bool isActive() { return mEntityStatus == ACTIVE ? true : false; }
    bool checkCollision(Entity* other) const { return isColliding(other); }

    void moveLeft()  { mMovement.x = -1; mDirection = LEFT;  }
    void moveRight() { mMovement.x =  1; mDirection = RIGHT; }
    void moveUp()    { mMovement.y = -1; mDirection = UP;    } 
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mColliderDimensions;    }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    EntityType  getEntityType()            const { return mEntityType;            }
    AIType      getAIType()                const { return mAIType;                }
    
    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }

    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;                 }
    void setAcceleration(Vector2 newAcceleration)
        { mAcceleration = newAcceleration;         }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    
    void loadToolTextures(const char* netPath, const char* rodPath, const char* wateringCanPath, const char* hoePath) {
        mTextureNet = LoadTexture(netPath);
        mTextureRod = LoadTexture(rodPath);
        mTextureWateringCan = LoadTexture(wateringCanPath);
        mTextureHoe = LoadTexture(hoePath);
    }

    void setColliderDimensions(Vector2 newDimensions) 
        { mColliderDimensions = newDimensions;     }
    void setSpriteSheetDimensions(Vector2 newDimensions) 
        { mSpriteSheetDimensions = newDimensions;  }
    void setSpeed(int newSpeed)
        { mSpeed  = newSpeed;                      }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;                  }
    void setAngle(float newAngle) 
        { mAngle = newAngle;                       }
    void setEntityType(EntityType entityType)
        { mEntityType = entityType;                }
    void setDirection(Direction newDirection)
    { 
        mDirection = newDirection;

        if (mTextureType == ATLAS) mAnimationIndices = mAnimationAtlas.at(mDirection);
    }
    void setAIType(AIType newType) { 
        mAIType = newType; 
        if (mAIType == AI_CIRCLE) {
            mAnchorPosition = mPosition;
        }
    }
    ToolType getTool() const { return mEquippedTool; }

    std::string interact(std::vector<Entity*>& worldEntities);
    std::string useTool(std::vector<Entity*>& worldEntities, Map* map);
    void sellItems();
    
    int getMoney() const { return mMoney; }
    int getInventorySize() const { return mInventory.size(); }
    const std::vector<Item>& getInventory() const { return mInventory; }
    static constexpr int MAX_INVENTORY_SIZE = 9;
    
    static int sGlobalMoney;
    static std::vector<Item> sGlobalInventory;
    static Vector2 sGlobalSpawnPosition;
    
    void savePlayerData() {
        sGlobalMoney = mMoney;
        sGlobalInventory = mInventory;
    }
    
    void loadPlayerData() {
        mMoney = sGlobalMoney;
        mInventory = sGlobalInventory;
    }

    static Vector2 getGlobalSpawnPosition() { return sGlobalSpawnPosition; }
    static void setGlobalSpawnPosition(Vector2 pos) { sGlobalSpawnPosition = pos; }

    
    struct CropSaveData {
        Vector2 position;
        CropType type;
        int growthStage;
        bool isWatered;
    };
    static std::vector<CropSaveData> sGlobalCrops;
    static int sGlobalDayCount;
    static float sGlobalTimeOfDay;
    CropType getCropType() const { return mCropType; }
    int getGrowthStage() const { return mGrowthStage; }
    void setGrowthStage(int stage) { mGrowthStage = stage; }

    void setShakeTimer(float time) { mShakeTimer = time; }
    
    void setDialogue(const std::vector<std::string>& lines) {
        mDialogueLines = lines;
        mDialogueIndex = 0;
    }

    std::string getNextDialogue() {
        if (mDialogueLines.empty()) return "";
        std::string line = mDialogueLines[mDialogueIndex];
        mDialogueIndex = (mDialogueIndex + 1) % mDialogueLines.size();
        return line;
    }
    
    void cycleTool() {
        int t = (int)mEquippedTool + 1;
        if (t > TOOL_WATERING_CAN) t = TOOL_NONE;
        mEquippedTool = (ToolType)t;
    }

    void setCropType(CropType type) { mCropType = type; }
    void setWatered(bool isWatered) { mIsWatered = isWatered; }
    bool isWatered() const { return mIsWatered; }
    void growCrop() {
        if (mCropType != CROP_NONE && mIsWatered) {
            if (mGrowthStage < 2) {
                mGrowthStage++;
            }
            mIsWatered = false;
        }
        else if (mCropType == CROP_NONE) {
             mIsWatered = false;
        }
    }
    
    static Sound sfxDoor;
    static Sound sfxTill;
    static Sound sfxWater;
    static Sound sfxCatch;
    static Sound sfxMoney;
    static Sound sfxPickup;

    static void loadEntitySounds() {
        sfxPickup     = LoadSound("assets/game/sfx_pickup.wav");
        sfxDoor       = LoadSound("assets/game/sfx_door.wav");
        sfxWater      = LoadSound("assets/game/sfx_water.wav");
        sfxTill       = LoadSound("assets/game/sfx_till.wav");
        sfxCatch      = LoadSound("assets/game/sfx_catch.wav");
        sfxMoney      = LoadSound("assets/game/sfx_money.wav");
    }

    static void unloadEntitySounds() {
        UnloadSound(sfxInteract);
        UnloadSound(sfxTreeShake);
        UnloadSound(sfxPickup);
        UnloadSound(sfxDoor);
        UnloadSound(sfxToolSwing);
        UnloadSound(sfxWater);
        UnloadSound(sfxTill);
        UnloadSound(sfxCatch);
        UnloadSound(sfxMoney);
    }

};

#endif