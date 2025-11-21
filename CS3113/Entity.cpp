#include <iostream>
#include "Entity.h"

Entity::Entity() 
    : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
      mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
      mScale {DEFAULT_SIZE, DEFAULT_SIZE},
      mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
      mTexture {0}, mTextureType {SINGLE}, mAngle {0.0f},
      mSpriteSheetDimensions {0.0f, 0.0f}, mDirection {RIGHT}, 
      mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
      mEntityType {MISC}, mAIStateTime{0.0f}, mAIStateDuration{2.0f}
{ 
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityType entityType) 
    : mPosition {position}, mVelocity {0.0f, 0.0f}, 
      mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
      mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
      mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}}, 
      mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
      mAngle {0.0f}, mEntityType {entityType}, 
      mAIStateTime{0.0f}, mAIStateDuration{2.0f}
{ 
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
    std::vector<int>> animationAtlas, EntityType entityType) 
    : mPosition {position}, mVelocity {0.0f, 0.0f}, 
      mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
      mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
      mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
      mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
      mAnimationIndices {animationAtlas.at(RIGHT)}, 
      mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
      mSpeed { DEFAULT_SPEED }, mEntityType {entityType},
      mAIStateTime{0.0f}, mAIStateDuration{2.0f}
{ 
}

Entity::~Entity() { UnloadTexture(mTexture); };

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
        // Don't collide with self
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

void Entity::AIWander(Map* map) 
{
    mAIStateTime += GetFrameTime();
    if (mAIStateTime >= mAIStateDuration)
    {
        mAIStateTime = 0.0f;
        mAIStateDuration = (float)GetRandomValue(10, 30) / 10.0f; // 1.0s to 3.0s

        if (mEntityType == NPC) 
        {
            int action = GetRandomValue(0, 100);
            if (action < 30) {
                mMovement = { 0.0f, 0.0f };
                mAIState = IDLE;
            } else {
                if (GetRandomValue(0, 1) == 0) moveLeft();
                else moveRight();
                mAIState = WALKING;
            }
        }
        else if (mEntityType == BUG || mEntityType == FISH) 
        {
            int action = GetRandomValue(0, 100);
            if (action < 20) {
                mMovement = { 0.0f, 0.0f };
            } else {
                float randX = (float)GetRandomValue(-10, 10) / 10.0f;
                float randY = (float)GetRandomValue(-10, 10) / 10.0f;
                mMovement = { randX, randY };
                Normalise(&mMovement);
            }
            
            if (fabs(mMovement.x) > fabs(mMovement.y)) {
                mDirection = (mMovement.x < 0) ? LEFT : RIGHT;
            } else {
                mDirection = (mMovement.y < 0) ? UP : DOWN;
            }
        }
    }

    if (map) {
        float padding = 50.0f; // Turn around before hitting the exact edge
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
    
    if (mEntityType == BUG && Vector2Length(mMovement) > 0.1f) {
        mAngle = atan2(mMovement.y, mMovement.x) * (180.0f / PI);
         mAngle += 90.0f; 
    } else {
        mAngle = 0.0f;
    }
}

void Entity::AIFollow(Entity *target)
{
    const float TURN_THRESHOLD = 120.0f;

    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 250.0f) mAIState = WALKING;
        break;

    case WALKING:
        {
            float playerX = target->getPosition().x;
            float myX = mPosition.x;

            if (mDirection == RIGHT || mDirection == RIGHT)
            {
                if (playerX < myX - TURN_THRESHOLD) moveLeft();
                else moveRight();
            }
            else if (mDirection == LEFT || mDirection == LEFT)
            {
                if (playerX > myX + TURN_THRESHOLD) moveRight();
                else moveLeft();
            }
            else
            {
                if (myX > playerX) moveLeft();
                else moveRight();
            }
        }
        break;
    
    default:
        break;
    }
}

void Entity::AIActivate(Entity *target, Map* map)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander(map);
        break;

    case FOLLOWER:
        AIFollow(target);
        break;

    case VERTICAL_FLYER:
        switch (mAIState)
        {
            case IDLE:
                if (Vector2Distance(mPosition, target->getPosition()) < 1200.0f)
                {
                    mAIState = WALKING;
                }
                mVelocity.y = 0.0f;
                break;
            case WALKING:
                if (Vector2Distance(mPosition, target->getPosition()) > 1250.0f)
                {
                    mAIState = IDLE;
                }
                break;
        }
        break;

    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*>& collidableEntities) 
{
    if (mEntityStatus == INACTIVE) return;

    // Pass 'map' to the AI
    if (mEntityType == NPC || mEntityType == BUG || mEntityType == FISH) {
        AIActivate(player, map);
    }

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

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            textureArea = {
                0.0f, 0.0f,

                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture, 
                mAnimationIndices[mCurrentFrameIndex], 
                mSpriteSheetDimensions.x, 
                mSpriteSheetDimensions.y
            );
        
        default: break;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset;

    if (mEntityType == PROP) {
        originOffset = {
            static_cast<float>(mScale.x) / 2.0f, 
            static_cast<float>(mScale.y) - (mColliderDimensions.y / 2.0f)
        };
    } else if (mEntityType == COLLECTIBLE || mEntityType == BUG || mEntityType == FISH) {
        originOffset = {
            static_cast<float>(mScale.x) / 2.0f,
            static_cast<float>(mScale.y) / 2.0f
        };
    } else if (mEntityType == PLAYER) {
        originOffset = {
            static_cast<float>(mScale.x) / 2.0f,
            static_cast<float>(mScale.y) / 2.0f + (mColliderDimensions.y * 1.5f) 
        };
    } else {
        originOffset = {
            static_cast<float>(mScale.x) / 2.0f,
            static_cast<float>(mScale.y) / 2.0f
        };
    }

    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );
}

void Entity::displayCollider() 
{
    // draw the collision box
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

    for (auto it = worldEntities.begin(); it != worldEntities.end(); )
    {
        Entity* e = *it;

        if (e == this) {
            ++it; 
            continue;
        }

        float entityHalfWidth  = e->getColliderDimensions().x / 2.0f;
        float entityHalfHeight = e->getColliderDimensions().y / 2.0f;
        float eLeft   = e->getPosition().x - entityHalfWidth;
        float eRight  = e->getPosition().x + entityHalfWidth;
        float eTop    = e->getPosition().y - entityHalfHeight;
        float eBottom = e->getPosition().y + entityHalfHeight;

        if (e->getEntityType() == COLLECTIBLE)
        {
            bool inProximity = (proxRight > eLeft) && (proxLeft < eRight) && 
                               (proxBottom > eTop) && (proxTop < eBottom);
            
            if (inProximity) {
                mInventory.push_back({ ITEM_APPLE, 10 });
                delete e;
                it = worldEntities.erase(it);
                break; 
            } else {
                ++it;
            }
        } else {
            bool inDirection = (dirRight > eLeft) && (dirLeft < eRight) && 
                               (dirBottom > eTop) && (dirTop < eBottom);

            if (inDirection)
            {
                if (e->getEntityType() == PROP && e->getScale().y == 48.0f)
                {
                    Entity* apple = new Entity(
                        {e->getPosition().x, e->getPosition().y + 15.0f}, 
                        {10.0f, 10.0f},
                        "assets/game/apple.png", 
                        COLLECTIBLE
                    );
                    apple->setColliderDimensions({8.0f, 8.0f});
                    worldEntities.push_back(apple);
                    break; 
                }
                else if (e->getEntityType() == NPC)
                {
                    if (mPosition.x < e->getPosition().x) e->setDirection(LEFT);
                    else e->setDirection(RIGHT);

                    output = e->getDialogue();
                    break;
                }
            }
            ++it;
        }
    }
    
    return output;
}

std::string Entity::useTool(std::vector<Entity*>& worldEntities)
{
    if (mEquippedTool == TOOL_NONE) return "";

    Vector2 reachPoint = mPosition;
    float reachDistance = 10.0f; 

    if (mDirection == LEFT || mDirection == LEFT_WALK)        reachPoint.x -= reachDistance;
    else if (mDirection == RIGHT || mDirection == RIGHT_WALK) reachPoint.x += reachDistance;
    else if (mDirection == UP || mDirection == UP_WALK)       reachPoint.y -= reachDistance;
    else if (mDirection == DOWN || mDirection == DOWN_WALK)   reachPoint.y += reachDistance;

    Rectangle toolHitbox = {
        reachPoint.x - 10, reachPoint.y - 10, 20, 20
    };

    if (mEquippedTool == TOOL_NET)
    {
        for (auto it = worldEntities.begin(); it != worldEntities.end(); )
        {
            Entity* e = *it;
            if (e->getEntityType() == BUG && e->isActive())
            {
                Rectangle targetRect = {
                    e->getPosition().x - e->getColliderDimensions().x/2,
                    e->getPosition().y - e->getColliderDimensions().y/2,
                    e->getColliderDimensions().x,
                    e->getColliderDimensions().y
                };

                if (CheckCollisionRecs(toolHitbox, targetRect))
                {
                    mInventory.push_back({ ITEM_BUTTERFLY, 150 }); 
                    delete e;
                    it = worldEntities.erase(it);
                    return "Caught a Bug!";
                }
            }
            ++it;
        }
    }

    else if (mEquippedTool == TOOL_ROD)
    {
        for (auto it = worldEntities.begin(); it != worldEntities.end(); )
        {
            Entity* e = *it;
            // FISH is now defined in Entity.h
            if (e->getEntityType() == FISH && e->isActive())
            {
                Rectangle targetRect = {
                    e->getPosition().x - e->getColliderDimensions().x/2,
                    e->getPosition().y - e->getColliderDimensions().y/2,
                    e->getColliderDimensions().x,
                    e->getColliderDimensions().y
                };

                if (CheckCollisionRecs(toolHitbox, targetRect))
                {
                    mInventory.push_back({ ITEM_BASS, 300 }); 
                    delete e;
                    it = worldEntities.erase(it);
                    return "Caught a Sea Bass! No wait--";
                }
            }
            ++it;
        }
    }
    
    return "";
}