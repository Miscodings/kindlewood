#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, RIGHT, UP, DOWN, LEFT_WALK, RIGHT_WALK, UP_WALK, DOWN_WALK };
enum EntityStatus { ACTIVE, INACTIVE                                      };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, PROP, COLLECTIBLE, BUG, FISH, NONE };
enum AIType       { WANDERER, FOLLOWER, VERTICAL_FLYER                    };
enum AIState      { WALKING, IDLE, FOLLOWING                              };
enum ToolType { TOOL_NONE, TOOL_NET, TOOL_AXE, TOOL_ROD };
enum ItemType { ITEM_NONE, ITEM_APPLE, ITEM_GEM, ITEM_BUTTERFLY, ITEM_BASS };

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

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;
    float mAIStateTime = 0.0f;
    float mAIStateDuration = 2.0f;

    bool isColliding(Entity *other) const;

    void checkCollisionY(const std::vector<Entity*>& collidableEntities);
    void checkCollisionX(const std::vector<Entity*>& collidableEntities);
    void checkCollisionX(Map *map);
    void checkCollisionY(Map *map);
    
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void AIActivate(Entity *target, Map* map); 
    void AIWander(Map* map);
    void AIFollow(Entity *target);

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
    AIState     getAIState()               const { return mAIState;               }

    
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
    void setAIState(AIState newState)
        { mAIState = newState;                     }
    void setAIType(AIType newType)
        { mAIType = newType;                       }
    ToolType getTool() const { return mEquippedTool; }

    std::string interact(std::vector<Entity*>& worldEntities);
    std::string useTool(std::vector<Entity*>& worldEntities); 
    void sellItems();
    
    int getMoney() const { return mMoney; }
    int getInventorySize() const { return mInventory.size(); }
    
    void setDialogue(std::string text) { mDialogueText = text; }
    std::string getDialogue() const { return mDialogueText; }
    
    void cycleTool() {
        int t = (int)mEquippedTool + 1;
        if (t > TOOL_ROD) t = TOOL_NONE;
        mEquippedTool = (ToolType)t;
    }
};

#endif