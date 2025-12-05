#include <iostream>
#include "Entity.h"

int Entity::sGlobalMoney = 0;
std::vector<Item> Entity::sGlobalInventory = {};
Vector2 Entity::sGlobalSpawnPosition = { -1.0f, -1.0f };
std::vector<Entity::CropSaveData> Entity::sGlobalCrops = {};
int Entity::sGlobalDayCount = 1;
float Entity::sGlobalTimeOfDay = 0.0f;

Sound Entity::sfxPickup = { 0 };
Sound Entity::sfxDoor = { 0 };
Sound Entity::sfxTill = { 0 };
Sound Entity::sfxCatch = { 0 };
Sound Entity::sfxMoney = { 0 };

Entity::Entity() 
    : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
      mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
      mScale {DEFAULT_SIZE, DEFAULT_SIZE},
      mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
      mTexture {0}, 
      mTextureNet {0}, mTextureRod {0}, mTextureWateringCan {0}, mTextureHoe {0},
      mTextureType {SINGLE}, mAngle {0.0f},
      mSpriteSheetDimensions {0.0f, 0.0f}, mDirection {RIGHT}, 
      mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0}, mEntityType {MISC},
      mIsFishing { false }, mFishOnHook { false }, 
      mFishingTimer { 0.0f }, mHookWindowTimer { 0.0f }, mShakeTimer { 0.0f },
      mCropType { CROP_NONE }, mGrowthStage { 0 }, mIsWatered { false }
{ 
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityType entityType) 
    : mPosition {position}, mVelocity {0.0f, 0.0f}, 
      mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
      mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
      mTextureNet {0}, mTextureRod {0}, mTextureWateringCan {0}, mTextureHoe {0},
      mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}}, 
      mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
      mAngle {0.0f}, mEntityType {entityType}, 
      mIsFishing { false }, mFishOnHook { false }, 
      mFishingTimer { 0.0f }, mHookWindowTimer { 0.0f }, mShakeTimer { 0.0f },
      mCropType { CROP_NONE }, mGrowthStage { 0 }, mIsWatered { false }
{ 
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
    std::vector<int>> animationAtlas, EntityType entityType) 
    : mPosition {position}, mVelocity {0.0f, 0.0f}, 
      mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
      mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
      mTextureNet {0}, mTextureRod {0}, mTextureWateringCan {0}, mTextureHoe {0},
      mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
      mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
      mAnimationIndices {animationAtlas.at(RIGHT)}, 
      mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
      mSpeed { DEFAULT_SPEED }, mEntityType {entityType},
      mIsFishing { false }, mFishOnHook { false }, 
      mFishingTimer { 0.0f }, mHookWindowTimer { 0.0f }, mShakeTimer { 0.0f },
      mCropType { CROP_NONE }, mGrowthStage { 0 }, mIsWatered { false }
{ 
}

Entity::~Entity() { 
    UnloadTexture(mTexture); 
    if (mTextureNet.id != 0) UnloadTexture(mTextureNet);
    if (mTextureRod.id != 0) UnloadTexture(mTextureRod);
    if (mTextureWateringCan.id != 0) UnloadTexture(mTextureWateringCan);
    if (mTextureHoe.id != 0) UnloadTexture(mTextureHoe);
};

void Entity::checkCollisionY(const std::vector<Entity*>& collidableEntities)
{
    for (Entity* collidableEntity : collidableEntities)
    {
        if (collidableEntity == this) continue; 

        EntityType type = collidableEntity->getEntityType();
        if (type == COLLECTIBLE || type == BUG || type == FISH) continue;

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                              (collidableEntity->mColliderDimensions.y / 2.0f));
                              
            if (mVelocity.y > 0) {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0) {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;
            }
        }
    }
}

void Entity::checkCollisionX(const std::vector<Entity*>& collidableEntities)
{
    for (Entity* collidableEntity : collidableEntities)
    {
        if (collidableEntity == this) continue;

        EntityType type = collidableEntity->getEntityType();
        if (type == COLLECTIBLE || type == BUG || type == FISH) continue;

        if (isColliding(collidableEntity))
        {            
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
                mIsCollidingLeft = true;
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 topCentreProbe    = { mPosition.x, mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topLeftProbe      = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topRightProbe     = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };

    Vector2 bottomCentreProbe = { mPosition.x, mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomLeftProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomRightProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) && mVelocity.y < 0.0f)
    {
        mPosition.y += yOverlap;
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }

    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap;
        mVelocity.y  = 0.0f;
        mIsCollidingBottom = true;
    } 
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;
    Vector2 leftCentreProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y };
    Vector2 rightCentreProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }

    if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01;
        mVelocity.x  = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const 
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);
    const float COLLISION_EPSILON = 0.5f;
    if (xDistance < -COLLISION_EPSILON && yDistance < -COLLISION_EPSILON) return true;
    return false;
}

void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::AIUpdate(Map* map)
{
    switch (mAIType)
    {
        case AI_WANDERER:
            updateWanderer(map);
            break;
        case AI_IDLER:
            updateIdler();
            break;
        case AI_CIRCLE:
            updateCircle();
            break;
    }
}

void Entity::updateWanderer(Map* map)
{
    mAITimer -= GetFrameTime();
    if (mAITimer <= 0.0f)
    {
        mAITimer = mAIDecisionTime;
        if (GetRandomValue(0, 100) < 20) { mAIDirection = {0, 0}; }
        else
        {
            float dx = (float)GetRandomValue(-10, 10) / 10.0f;
            float dy = (float)GetRandomValue(-10, 10) / 10.0f;
            mAIDirection = Vector2Normalize({dx, dy});
        }
    }
    mMovement = mAIDirection;

    if (map) { 
        float padding = 50.0f; 
        float mapL = map->getLeftBoundary() + padding; 
        float mapR = map->getRightBoundary() - padding; 
        float mapT = map->getTopBoundary() + padding; 
        float mapB = map->getBottomBoundary() - padding; 
        if (mPosition.x < mapL) mMovement.x = 0.5f; 
        if (mPosition.x > mapR) mMovement.x = -0.5f; 
        if (mEntityType == BUG || mEntityType == FISH) { 
            if (mPosition.y < mapT) mMovement.y = 0.5f; 
            if (mPosition.y > mapB) mMovement.y = -0.5f; 
        } 
    }
}

void Entity::updateIdler()
{
    mMovement = {0, 0};
    mAITimer -= GetFrameTime();
    if (mAITimer <= 0.0f)
    {
        mAITimer = mAIDecisionTime * 2.0f;

        int d = GetRandomValue(0, 3);
        switch (d)
        {
            case 0: mDirection = LEFT; break;
            case 1: mDirection = RIGHT; break;
            case 2: mDirection = UP; break;
            case 3: mDirection = DOWN; break;
        }
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*>& collidableEntities) 
{
    if (mEntityStatus == INACTIVE) return;

    if (mShakeTimer > 0.0f) { mShakeTimer -= deltaTime; }

    if (mIsFishing)
    {
        mMovement = { 0.0f, 0.0f };
        mVelocity = { 0.0f, 0.0f };

        if (!mFishOnHook)
        {
            mFishingTimer -= deltaTime;
            if (mFishingTimer <= 0.0f)
            {
                mFishOnHook = true;
                mHookWindowTimer = 1.5f;
            }
        } else {
            mHookWindowTimer -= deltaTime;
            if (mHookWindowTimer <= 0.0f)
            {
                mIsFishing = false;
                mFishOnHook = false;
            }
        }
        return; 
    }

    if (mEntityType == NPC || mEntityType == BUG || mEntityType == FISH) { AIUpdate(map); }

    resetColliderFlags();

    mVelocity.x = mMovement.x * mSpeed;
    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y = mMovement.y * mSpeed;
    mVelocity.y += mAcceleration.y * deltaTime;

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities);
    checkCollisionX(map);

    if (mMovement.x < 0)      mDirection = LEFT;
    else if (mMovement.x > 0) mDirection = RIGHT;
    else if (mMovement.y < 0) mDirection = UP;
    else if (mMovement.y > 0) mDirection = DOWN;

    if (Vector2Length(mVelocity) > 1.0f) 
    {
        if (mDirection == LEFT)       mDirection = LEFT_WALK;
        else if (mDirection == RIGHT) mDirection = RIGHT_WALK;
        else if (mDirection == UP)    mDirection = UP_WALK;
        else if (mDirection == DOWN)  mDirection = DOWN_WALK;
    }
    else 
    {
        if (mDirection == LEFT_WALK)       mDirection = LEFT;
        else if (mDirection == RIGHT_WALK) mDirection = RIGHT;
        else if (mDirection == UP_WALK)    mDirection = UP;
        else if (mDirection == DOWN_WALK)  mDirection = DOWN;
    }

    if (mTextureType == ATLAS) {
        auto it = mAnimationAtlas.find(mDirection);
        if (it != mAnimationAtlas.end()) {
            mAnimationIndices = it->second;
            animate(deltaTime);
        }
    }
}

void Entity::drawTool()
{
    if (mEquippedTool == TOOL_NONE) return;

    Texture2D* currentToolTex = nullptr;
    if      (mEquippedTool == TOOL_NET)             currentToolTex = &mTextureNet;
    else if (mEquippedTool == TOOL_ROD)             currentToolTex = &mTextureRod;
    else if (mEquippedTool == TOOL_WATERING_CAN)    currentToolTex = &mTextureWateringCan;
    else if (mEquippedTool == TOOL_HOE)             currentToolTex = &mTextureHoe;

    if (!currentToolTex || currentToolTex->id == 0) return;

    Vector2 toolOffset = { 0.0f, 0.0f };
    float toolSize     = 12.0f; 

    Vector2 dir = mMovement;

    if (dir.x == 0.0f && dir.y == 0.0f) {
        if (mDirection == LEFT)      dir.x = -1.0f;
        if (mDirection == RIGHT)     dir.x =  1.0f;
        if (mDirection == UP)        dir.y = -1.0f;
        if (mDirection == DOWN)      dir.y =  1.0f;
    }

    if (dir.x < 0.0f) {
        toolOffset = { -16.0f, -8.0f };
    }
    else if (dir.x > 0.0f) {
        toolOffset = { 16.0f, -8.0f };
    }
    else if (dir.y < 0.0f) {
        toolOffset = { 0.0f, -24.0f };
    }
    else if (dir.y > 0.0f) {
        toolOffset = { 0.0f, 6.0f };
    }

    if (mFishOnHook) {
        toolOffset.x += GetRandomValue(-2, 2);
        toolOffset.y += GetRandomValue(-2, 2);
        DrawText("!", mPosition.x + toolOffset.x, mPosition.y + toolOffset.y - 10, 20, RED);
    }

    Vector2 drawPos = { mPosition.x + toolOffset.x, mPosition.y + toolOffset.y };

    Rectangle sourceRec = { 0.0f, 0.0f, (float)currentToolTex->width, (float)currentToolTex->height };
    Rectangle destRec = { drawPos.x, drawPos.y, toolSize, toolSize };
    Vector2 origin = { toolSize / 2.0f, toolSize / 2.0f }; 

    DrawTexturePro(*currentToolTex, sourceRec, destRec, origin, 0, WHITE);
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;
    float shakeX = 0.0f;
    float shakeY = 0.0f;

    if (mShakeTimer > 0.0f) {
        shakeX = (float)GetRandomValue(-2, 2);
        shakeY = (float)GetRandomValue(-2, 2);
    }

    Rectangle destinationArea = { 
        mPosition.x + shakeX,
        mPosition.y + shakeY,
        static_cast<float>(mScale.x), 
        static_cast<float>(mScale.y) 
    };
    Color tint = WHITE;
    
    Vector2 originOffset = { 0.0f, 0.0f };
    Rectangle textureArea;

    if (mEntityType == CROP) {
        float tileDim = 32.0f;
        if (mIsWatered) { tint = { 180, 180, 220, 255 }; }
        
        int row = 0; int col = 0;

        if (mCropType == CROP_NONE) { row = 0; col = 0;} 
        else { row = (int)mCropType; col = mGrowthStage; }
        
        textureArea = { 
            col * tileDim, 
            row * tileDim, 
            tileDim, 
            tileDim 
        };
        
        originOffset = { static_cast<float>(mScale.x) / 2.0f,  static_cast<float>(mScale.y) / 2.0f };

    } else {
        switch (mTextureType)
        {
            case SINGLE:
                textureArea = { 0.0f, 0.0f, static_cast<float>(mTexture.width), static_cast<float>(mTexture.height) };
                break;
            case ATLAS:
                textureArea = getUVRectangle(&mTexture, mAnimationIndices[mCurrentFrameIndex], mSpriteSheetDimensions.x, mSpriteSheetDimensions.y);
                break;
            default: break;
        }

        if (mEntityType == PROP) {
            originOffset = { static_cast<float>(mScale.x) / 2.0f, static_cast<float>(mScale.y) - (mColliderDimensions.y / 2.0f) };
        } else if (mEntityType == PLAYER) {
            originOffset = { static_cast<float>(mScale.x) / 2.0f, static_cast<float>(mScale.y) / 2.0f + (mColliderDimensions.y * 1.5f) };
        } else {
            originOffset = { static_cast<float>(mScale.x) / 2.0f, static_cast<float>(mScale.y) / 2.0f + 10.0f };
        }

        bool holdingTool = (mEntityType == PLAYER && mEquippedTool != TOOL_NONE);

        if (holdingTool) {
            drawTool();
        }
    }

    DrawTexturePro(mTexture, textureArea, destinationArea, originOffset, mAngle, tint);
}

void Entity::displayCollider() 
{
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,  
        mPosition.y - mColliderDimensions.y / 2.0f,  
        mColliderDimensions.x,                        
        mColliderDimensions.y                        
    };

    DrawRectangleLines(
        colliderBox.x,
        colliderBox.y,
        colliderBox.width,
        colliderBox.height,
        GREEN
    );
}

void Entity::sellItems()
{
    if (mInventory.empty()) return;

    int totalValue = 0;
    for (const auto& item : mInventory) {
        totalValue += item.value;
    }

    mMoney += totalValue;
    mInventory.clear();
    PlaySound(sfxMoney);

    std::cout << "Sold items for $" << totalValue << ". Total: $" << mMoney << std::endl;
}

std::string Entity::interact(std::vector<Entity*>& worldEntities)
{
    std::string output = "";

    float sensorSize = 10.0f; 
    float reach = 10.0f;

    Vector2 dirCenter = mPosition;
    if (mDirection == LEFT || mDirection == LEFT_WALK)        dirCenter.x -= reach;
    else if (mDirection == RIGHT || mDirection == RIGHT_WALK) dirCenter.x += reach;
    else if (mDirection == UP || mDirection == UP_WALK)       dirCenter.y -= reach;
    else if (mDirection == DOWN || mDirection == DOWN_WALK)   dirCenter.y += reach;

    float dirLeft   = dirCenter.x - (sensorSize / 2.0f);
    float dirRight  = dirCenter.x + (sensorSize / 2.0f);
    float dirTop    = dirCenter.y - (sensorSize / 2.0f);
    float dirBottom = dirCenter.y + (sensorSize / 2.0f);

    float proxSize = 10.0f; 
    float proxLeft   = mPosition.x - (proxSize / 2.0f);
    float proxRight  = mPosition.x + (proxSize / 2.0f);
    float proxTop    = mPosition.y - (proxSize / 2.0f);
    float proxBottom = mPosition.y + (proxSize / 2.0f);

    for (auto it = worldEntities.begin(); it != worldEntities.end(); ) {
        Entity* e = *it;
        if (e == this) { ++it; continue; }

        float eHalfW = e->getColliderDimensions().x / 2.0f;
        float eHalfH = e->getColliderDimensions().y / 2.0f;
        float eLeft   = e->getPosition().x - eHalfW;
        float eRight  = e->getPosition().x + eHalfW;
        float eTop    = e->getPosition().y - eHalfH;
        float eBottom = e->getPosition().y + eHalfH;

        if (e->getEntityType() == COLLECTIBLE) {
            bool inProximity = (proxRight > eLeft) && (proxLeft < eRight) && 
                               (proxBottom > eTop) && (proxTop < eBottom);
            
            if (inProximity) {
                if (mInventory.size() >= MAX_INVENTORY_SIZE) {
                    output = "Inventory Full!";
                    ++it; continue; 
                }
                mInventory.push_back({ ITEM_APPLE, 10 });
                delete e;
                it = worldEntities.erase(it);
                PlaySound(sfxPickup);
                output = "Picked up an Apple";
                break; 
            } else { ++it; }
        } else {
            bool inDirection = (dirRight > eLeft) && (dirLeft < eRight) && 
                               (dirBottom > eTop) && (dirTop < eBottom);

            if (inDirection) {
                if (e->getEntityType() == PROP && e->getScale().y == 48.0f) {
                    float randX = (float)GetRandomValue(-20, 20);
                    float randY = (float)GetRandomValue(10, 30);
                    Entity* apple = new Entity(
                        {e->getPosition().x + randX, e->getPosition().y + randY}, 
                        {10.0f, 10.0f}, "assets/game/apple.png", COLLECTIBLE
                    );
                    apple->setColliderDimensions({8.0f, 8.0f});
                    apple->setShakeTimer(0.5f);
                    worldEntities.push_back(apple);
                    break; 
                } else if (e->getEntityType() == NPC) {
                    if (mPosition.x < e->getPosition().x) e->setDirection(LEFT);
                    else e->setDirection(RIGHT);
                    output = e->getNextDialogue();
                    if (output.find("Shopkeeper|") == 0) {
                        sellItems();
                    }
                    break;
                } else if (e->getEntityType() == CROP) {
                    if (e->mGrowthStage == 2 && e->mCropType != CROP_NONE) 
                    {
                        if (mInventory.size() >= MAX_INVENTORY_SIZE) {
                            output = "Inventory Full!";
                        } else {
                            ItemType produce = ITEM_NONE;
                            switch (e->mCropType) {
                                case CROP_CORN:        produce = ITEM_CORN; break;
                                case CROP_TURNIP:      produce = ITEM_TURNIP; break;
                                case CROP_CAULIFLOWER: produce = ITEM_CAULIFLOWER; break;
                                case CROP_PEPPER:      produce = ITEM_PEPPER; break;
                                case CROP_PINEAPPLE:   produce = ITEM_PINEAPPLE; break;
                                case CROP_SQUASH:      produce = ITEM_SQUASH; break;
                                case CROP_PUMPKIN:     produce = ITEM_PUMPKIN; break;
                                case CROP_CARROT:      produce = ITEM_CARROT; break;
                                default: break;
                            }

                            if (produce != ITEM_NONE) {
                                mInventory.push_back({ produce, 50 });
                                e->setCropType(CROP_NONE); // Reset to empty soil
                                e->mGrowthStage = 0;
                                e->setWatered(false);
                                output = "Harvested!";
                                PlaySound(sfxPickup);
                            }
                        }
                        break; 
                    } else if (e->mCropType == CROP_NONE) {
                        int seedIndex = -1;
                        CropType typeToPlant = CROP_NONE;

                        for (int i = 0; i < mInventory.size(); i++) {
                            switch (mInventory[i].type) {
                                case SEEDS_CORN:        seedIndex = i; typeToPlant = CROP_CORN; break;
                                case SEEDS_TURNIP:      seedIndex = i; typeToPlant = CROP_TURNIP; break;
                                case SEEDS_CAULIFLOWER: seedIndex = i; typeToPlant = CROP_CAULIFLOWER; break;
                                case SEEDS_PEPPER:      seedIndex = i; typeToPlant = CROP_PEPPER; break;
                                case SEEDS_PINEAPPLE:   seedIndex = i; typeToPlant = CROP_PINEAPPLE; break;
                                case SEEDS_SQUASH:      seedIndex = i; typeToPlant = CROP_SQUASH; break;
                                case SEEDS_PUMPKIN:     seedIndex = i; typeToPlant = CROP_PUMPKIN; break;
                                case SEEDS_CARROT:      seedIndex = i; typeToPlant = CROP_CARROT; break;
                                default: break;
                            }
                            if (seedIndex != -1) break;
                        }

                        if (seedIndex != -1) {
                            e->setCropType(typeToPlant);
                            e->mGrowthStage = 0; 
                            mInventory.erase(mInventory.begin() + seedIndex);
                            output = "Planted seeds.";
                            PlaySound(sfxTill);
                        } else {
                            if (mEquippedTool == TOOL_NONE) {
                                delete e;
                                it = worldEntities.erase(it);
                                output = "Covered soil.";
                                return output; 
                                PlaySound(sfxTill);
                            } else {
                                output = "No seeds in bag.";
                                break;
                            }
                        }
                    }
                } else if (e->getEntityType() == PROP && !e->getNextDialogue().empty()) {
                    std::string data = e->getNextDialogue();
                    
                    if (data.find("Buy|") == 0) {
                        std::string clean = data.substr(4);
                        
                        size_t firstPipe = clean.find('|');
                        size_t secondPipe = clean.find('|', firstPipe + 1);
                        
                        std::string seedName = clean.substr(0, firstPipe);
                        int cost = std::stoi(clean.substr(firstPipe + 1, secondPipe - firstPipe - 1));
                        int enumId = std::stoi(clean.substr(secondPipe + 1));

                        if (mMoney >= cost) {
                            if (mInventory.size() >= MAX_INVENTORY_SIZE) {
                                output = "Inventory Full!";
                            } else {
                                mMoney -= cost;
                                mInventory.push_back({ (ItemType)enumId, 0 });
                                output = "Bought " + seedName;
                                PlaySound(sfxMoney);
                            }
                        } else {
                            output = "Not enough money!";
                        }
                        break; 
                    }
                    else if (data == "ACTION_ENTER_HOUSE") {
                        PlaySound(sfxDoor);
                        output = "ACTION_ENTER_HOUSE"; // Signal to Scene
                        break;
                    }
                }
            }
            ++it;
        }
    }
    return output;
}

std::string Entity::useTool(std::vector<Entity*>& worldEntities, Map* map)
{
    if (mEquippedTool == TOOL_NONE) return "";

    Vector2 reachPoint = mPosition;
    float reachDistance = 10.0f;

    if      (mDirection == LEFT  || mDirection == LEFT_WALK)   reachPoint.x -= reachDistance;
    else if (mDirection == RIGHT || mDirection == RIGHT_WALK) reachPoint.x += reachDistance;
    else if (mDirection == UP    || mDirection == UP_WALK)       reachPoint.y -= reachDistance;
    else if (mDirection == DOWN  || mDirection == DOWN_WALK)   reachPoint.y += reachDistance;

    Rectangle toolHitbox = { reachPoint.x - 10, reachPoint.y - 10, 20, 20 };

    if (mEquippedTool == TOOL_NET) {
        for (auto it = worldEntities.begin(); it != worldEntities.end(); ) {
            Entity* e = *it;
            if (e->getEntityType() == BUG && e->isActive()) {
                Rectangle targetRect = {
                    e->getPosition().x - e->getColliderDimensions().x/2,
                    e->getPosition().y - e->getColliderDimensions().y/2,
                    e->getColliderDimensions().x,
                    e->getColliderDimensions().y
                };

                if (CheckCollisionRecs(toolHitbox, targetRect)) {
                    if (mInventory.size() >= MAX_INVENTORY_SIZE) { return "Inventory Full!"; }
                    mInventory.push_back({ ITEM_BUTTERFLY, 150 }); 
                    delete e;
                    it = worldEntities.erase(it);
                    PlaySound(sfxCatch);
                    return "Caught a Bug!";
                }
            }
            ++it;
        }
    }

    else if (mEquippedTool == TOOL_ROD) {
        if (!mIsFishing) {
            Vector2 castPos = mPosition;
            float castDist = 30.0f; 

            if (mDirection == LEFT || mDirection == LEFT_WALK)        castPos.x -= castDist;
            else if (mDirection == RIGHT || mDirection == RIGHT_WALK) castPos.x += castDist;
            else if (mDirection == UP || mDirection == UP_WALK)       castPos.y -= castDist;
            else if (mDirection == DOWN || mDirection == DOWN_WALK)   castPos.y += castDist;

            float dummy = 0.0f;
            bool isWater = (map != nullptr) && map->isSolidTileAt(castPos, &dummy, &dummy);

            if (isWater) 
            {
                mIsFishing = true;
                mFishOnHook = false;
                mFishingTimer = (float)GetRandomValue(20, 50) / 10.0f; 
                return "Casting...";
            } 
            else  { return "Not enough water here."; }
        } else {
            mIsFishing = false;

            if (mFishOnHook)
            {
                if (mInventory.size() >= MAX_INVENTORY_SIZE) {
                    mFishOnHook = false;
                    return "Inventory Full! The fish got away...";
                }

                mInventory.push_back({ ITEM_BASS, 300 }); 
                mFishOnHook = false;
                int prob = GetRandomValue(0, 15);
                PlaySound(sfxCatch);

                if (prob == 1) {
                    return "I caught a bitterling! What's it so bitter about?";
                } else if (prob == 2) {
                    return "I caught a pale chub! It could use some sun!";
                } else if (prob == 3) {
                    return "I caught a crucian carp! What a crucial catch!";
                } else if (prob == 4) {
                    return "I caught a dace! Wrong place, Mr. Dace.";
                } else if (prob == 5) {
                    return "I caught a carp! I really seized the diem!";
                } else if (prob == 6) {
                    return "I caught a koi! Can't play koi with me!";
                } else if (prob == 7) {
                    return "I caught a goldfish! I'm swimming in riches!";
                } else if (prob == 8) {
                    return "I caught an eel! Things just got REAL!"	;
                } else if (prob == 9) {
                    return "I caught a catfish! Purr-fect catch!";
                } else if (prob == 10) {
                    return "I caught a bluegill! There's a lot in this ville!"	;
                } else if (prob == 11) {
                    return "I caught a largemouth bass! I'm the bass-ically the best!";
                } else if (prob == 12) {
                    return "I caught a rainbow trout! That's what I'm talking about!"	;
                } else if (prob == 13) {
                    return "I caught a trout! What a great tr-outcome!";
                } else if (prob == 14) {
                    return "I caught a salmon! I'm on a roll!";
                } else if (prob == 15) {
                    return "I caught a piranha! Or did it catch ME?!";
                }
            }
            else { return "Pulled too early!"; }
        }
    } else if (mEquippedTool == TOOL_HOE) {
        Vector2 target = mPosition;
        if (mDirection == LEFT) target.x -= 16;
        if (mDirection == RIGHT) target.x += 16;
        if (mDirection == UP) target.y -= 16;
        if (mDirection == DOWN) target.y += 16;
        
        int gridX = (int)((target.x) / 16.0f) * 16;
        int gridY = (int)((target.y) / 16.0f) * 16;
        
        float dummy = 0;
        if (map->isSolidTileAt({(float)gridX + 8, (float)gridY + 8}, &dummy, &dummy)) {
             return "Can't till here.";
        }

        bool occupied = false;
        for (Entity* e : worldEntities) {
            if (Vector2Distance(e->getPosition(), {(float)gridX + 8, (float)gridY + 8}) < 12.0f) { occupied = true; break; }
            Rectangle entRect = {
                e->getPosition().x - e->getColliderDimensions().x/2,
                e->getPosition().y - e->getColliderDimensions().y/2,
                e->getColliderDimensions().x,
                e->getColliderDimensions().y
            };
            Rectangle hoeRect = { (float)gridX + 4, (float)gridY + 4, 8, 8 };
            
            if (CheckCollisionRecs(entRect, hoeRect)) {
                occupied = true; break;
            }
        }

        if (!occupied) {
            Entity* crop = new Entity(
                {(float)gridX + 8, (float)gridY + 8},
                {16.0f, 16.0f},
                "assets/game/Tileset_Farming.png",
                CROP
            );
            crop->setColliderDimensions({14, 14});
            worldEntities.push_back(crop);
            PlaySound(sfxTill);
            return "Tilled soil.";
        }
        return "Blocked.";

    } else if (mEquippedTool == TOOL_WATERING_CAN) {
        bool wateredAny = false;
        for (Entity* e : worldEntities) {
            if (e->getEntityType() == CROP) 
            {
                Rectangle cropRect = {
                    e->getPosition().x - e->getColliderDimensions().x / 2.0f,
                    e->getPosition().y - e->getColliderDimensions().y / 2.0f,
                    e->getColliderDimensions().x,
                    e->getColliderDimensions().y
                };

                if (CheckCollisionRecs(toolHitbox, cropRect)) {
                    e->setWatered(true);
                    wateredAny = true; 
                }
            }
        }
        if (wateredAny) return "Watered.";
    } 
    return "";
}

void Entity::updateCircle()
{
    mCircleAngle += GetFrameTime() * 2.0f; 
    mMovement.x = -sinf(mCircleAngle);
    mMovement.y = cosf(mCircleAngle);
}