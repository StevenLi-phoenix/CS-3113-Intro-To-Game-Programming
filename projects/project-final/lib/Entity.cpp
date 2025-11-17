#include "Entity.h"

Entity::Entity() : 
    mParent {nullptr},
    mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
    mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
    mScale {EntityConstants::DEFAULT_SIZE, EntityConstants::DEFAULT_SIZE},
    mColliderDimensions {EntityConstants::DEFAULT_SIZE, EntityConstants::DEFAULT_SIZE}, 
    mTexture {0}, mTextureType {EntityConstants::SINGLE}, mAngle {0.0f},
    mSpriteSheetDimensions {},
    mAnimationIndices {}, mFrameSpeed {0},
    mIsActive {true}, enableControl {false}, mAIActive {false}, canCollide {true}, mIsTextureAtlas {false}, mIsHorizontalFlipped {false}, mIsVerticalFlipped {false},
    mSpeed {EntityConstants::DEFAULT_SPEED},
    mIsCollidingTop {false}, mIsCollidingBottom {false}, mIsCollidingRight {false}, mIsCollidingLeft {false}
{
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath) : 
    mParent {nullptr},
    mPosition {position}, mMovement {0.0f, 0.0f}, 
    mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
    mScale {scale}, mColliderDimensions {scale}, 
    mTexture {LoadTexture(textureFilepath)}, mTextureType {EntityConstants::SINGLE}, mAngle {0.0f},
    mSpriteSheetDimensions {},
    mAnimationIndices {}, mFrameSpeed {0},
    mIsActive {true}, enableControl {false}, mAIActive {false}, canCollide {true}, mIsTextureAtlas {false}, mIsHorizontalFlipped {false}, mIsVerticalFlipped {false},
    mSpeed {EntityConstants::DEFAULT_SPEED},
    mIsCollidingTop {false}, mIsCollidingBottom {false}, mIsCollidingRight {false}, mIsCollidingLeft {false}
{
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityConstants::TextureType textureType, Vector2 spriteSheetDimensions,  std::vector<int> animationIndices) : 
    mParent {nullptr},
    mPosition {position}, mMovement {0.0f, 0.0f}, 
    mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
    mScale {scale}, mColliderDimensions {scale}, 
    mTexture {LoadTexture(textureFilepath)}, mTextureType {textureType}, mAngle {0.0f},
    mSpriteSheetDimensions {spriteSheetDimensions}, 
    mAnimationIndices {animationIndices}, mFrameSpeed {0},
    mIsActive {true}, enableControl {false}, mAIActive {false}, canCollide {true}, mIsTextureAtlas {false}, mIsHorizontalFlipped {false}, mIsVerticalFlipped {false},
    mSpeed {EntityConstants::DEFAULT_SPEED},
    mIsCollidingTop {false}, mIsCollidingBottom {false}, mIsCollidingRight {false}, mIsCollidingLeft {false}
{
}

Entity::~Entity()
{
    UnloadTexture(mTexture);
}

bool Entity::isColliding(Entity *other) const
{
    if (!other->mIsActive || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

bool Entity::isColliding(Map *map) const
{
    return mColliderDimensions.x > 0.0f && mColliderDimensions.y > 0.0f &&
           map->isSolidTileAt(mPosition);
}

bool Entity::intersects(const Entity &other) const
{
    if (&other == this || !other.mIsActive) return false;

    float xDistance = fabs(mPosition.x - other.getPosition().x) - 
        ((mColliderDimensions.x + other.getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other.getPosition().y) - 
        ((mColliderDimensions.y + other.getColliderDimensions().y) / 2.0f);
    return (xDistance < 0.0f && yDistance < 0.0f);
}

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *entity = &collidableEntities[i];
        if (entity == this) continue;
        if (isColliding(entity) && canCollide)
        {
            float yDistance = mVelocity.y - entity->mVelocity.y;
            float yABSDistance = fabs(yDistance);
            float yOverlap  = fabs(yABSDistance - (mColliderDimensions.y / 2.0f) - 
                              (entity->mColliderDimensions.y / 2.0f));
            
            if (yDistance > 0.0f) 
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (yDistance < 0.0f) 
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *entity = &collidableEntities[i];
        if (entity == this) continue;
        if (isColliding(entity) && canCollide)
        {
            float xDistance = mVelocity.x - entity->mVelocity.x;
            float xABSDistance = fabs(xDistance);
            float xOverlap  = fabs(xABSDistance - (mColliderDimensions.x / 2.0f) - 
                              (entity->mColliderDimensions.x / 2.0f));
            
            if (xDistance > 0.0f) 
            {
                mPosition.x -= xOverlap;
                mVelocity.x  = 0;
                mIsCollidingRight = true;
            } else if (xDistance < 0.0f) 
            {
                mPosition.x += xOverlap;
                mVelocity.x  = 0;
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
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) && mVelocity.y < 0.0f)
    {
        mPosition.y += yOverlap;
        mVelocity.y  = 0;
        mIsCollidingTop = true;
    }

    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap;
        mVelocity.y  = 0;
        mIsCollidingBottom = true;
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 leftTopProbe      = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 leftCentreProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y };
    Vector2 leftBottomProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };
    
    Vector2 rightTopProbe     = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 rightCentreProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y };
    Vector2 rightBottomProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;
    
    if ((map->isSolidTileAt(leftTopProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap)) && mVelocity.x < 0.0f)
    {
        mPosition.x += xOverlap;
        mVelocity.x  = 0;
        mIsCollidingLeft = true;
    }
    
    if ((map->isSolidTileAt(rightTopProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap)) && mVelocity.x > 0.0f)
    {
        mPosition.x -= xOverlap;
        mVelocity.x  = 0;
        mIsCollidingRight = true;
    }
}

void Entity::animate(float deltaTime)
{
    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;
        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, Entity *collidableEntities, int collisionCheckCount)
{
    if (!mIsActive) return;

    if (mAIActive) AIupdate(deltaTime);

    resetColliderFlags();

    mMovement += mAcceleration * deltaTime;
    mVelocity += mMovement * deltaTime;
    mPosition += mVelocity * deltaTime;

    // resolve collisions
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionY(map);
    checkCollisionX(map);

    if (mIsTextureAtlas && !mAnimationIndices.empty() && mFrameSpeed > 0) animate(deltaTime);

}

void Entity::render()
{
    if (!mIsActive) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case EntityConstants::SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case EntityConstants::ATLAS:
            textureArea = getUVRectangle(&mTexture, mCurrentFrameIndex, mSpriteSheetDimensions.y, mSpriteSheetDimensions.x);
            break;
        default: break;
    }

    Rectangle sourceArea = textureArea;

    if (mIsHorizontalFlipped)
    {
        sourceArea.width = -sourceArea.width;
    }

    if (mIsVerticalFlipped)
    {
        sourceArea.height = -sourceArea.height;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 textureOrigin = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(mTexture, sourceArea, destinationArea, textureOrigin, mAngle, WHITE);
}

void Entity::displayCollider()
{
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,
        mPosition.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };
    DrawRectangleLines(colliderBox.x, colliderBox.y, colliderBox.width, colliderBox.height, GREEN);
}

void Entity::AIupdate(float deltaTime)
{
    // Default implementation does nothing
    // Override this in derived classes for AI behavior
}