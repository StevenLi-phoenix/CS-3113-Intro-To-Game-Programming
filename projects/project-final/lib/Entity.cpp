#include "Entity.h"
#include <utility>

Entity::Entity() : 
    mParent {nullptr},
    mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
    mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
    mScale {EntityConstants::DEFAULT_SIZE, EntityConstants::DEFAULT_SIZE},
    mTextureOffset {0.0f, 0.0f},
    mColliderDimensions {EntityConstants::DEFAULT_SIZE, EntityConstants::DEFAULT_SIZE}, 
    mTexture {0}, mOwnsTexture {true}, mTextureType {EntityConstants::SINGLE}, mAngle {0.0f},
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
    mScale {scale}, mTextureOffset {0.0f, 0.0f}, mColliderDimensions {scale}, 
    mTexture {LoadTexture(textureFilepath)}, mOwnsTexture {true}, mTextureType {EntityConstants::SINGLE}, mAngle {0.0f},
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
    mScale {scale}, mTextureOffset {0.0f, 0.0f}, mColliderDimensions {scale}, 
    mTexture {LoadTexture(textureFilepath)}, mOwnsTexture {true}, mTextureType {textureType}, mAngle {0.0f},
    mSpriteSheetDimensions {spriteSheetDimensions}, 
    mAnimationIndices {animationIndices}, mFrameSpeed {0},
    mIsActive {true}, enableControl {false}, mAIActive {false}, canCollide {true}, mIsTextureAtlas {false}, mIsHorizontalFlipped {false}, mIsVerticalFlipped {false},
    mSpeed {EntityConstants::DEFAULT_SPEED},
    mIsCollidingTop {false}, mIsCollidingBottom {false}, mIsCollidingRight {false}, mIsCollidingLeft {false}
{
}

Entity::Entity(Entity&& other) noexcept :
    mParent {other.mParent},
    mPosition {other.mPosition},
    mMovement {other.mMovement},
    mVelocity {other.mVelocity},
    mAcceleration {other.mAcceleration},
    mScale {other.mScale},
    mTextureOffset {other.mTextureOffset},
    mColliderDimensions {other.mColliderDimensions},
    mTexture {other.mTexture},
    mOwnsTexture {other.mOwnsTexture},
    mTextureType {other.mTextureType},
    mAngle {other.mAngle},
    mSpriteSheetDimensions {other.mSpriteSheetDimensions},
    mAnimationIndices {std::move(other.mAnimationIndices)},
    mFrameSpeed {other.mFrameSpeed},
    mCurrentFrameIndex {other.mCurrentFrameIndex},
    mAnimationTime {other.mAnimationTime},
    mIsJumping {other.mIsJumping},
    mJumpingPower {other.mJumpingPower},
    mIsActive {other.mIsActive},
    enableControl {other.enableControl},
    mAIActive {other.mAIActive},
    canCollide {other.canCollide},
    mIsTextureAtlas {other.mIsTextureAtlas},
    mIsHorizontalFlipped {other.mIsHorizontalFlipped},
    mIsVerticalFlipped {other.mIsVerticalFlipped},
    mSpeed {other.mSpeed},
    mIsCollidingTop {other.mIsCollidingTop},
    mIsCollidingBottom {other.mIsCollidingBottom},
    mIsCollidingRight {other.mIsCollidingRight},
    mIsCollidingLeft {other.mIsCollidingLeft}
{
    other.mParent = nullptr;
    other.mTexture = Texture2D{};
    other.mAnimationIndices.clear();
    other.mIsActive = false;
    other.mOwnsTexture = false;
    other.mTextureOffset = {0.0f, 0.0f};
}

Entity& Entity::operator=(Entity&& other) noexcept
{
    if (this == &other) return *this;

    if (mOwnsTexture && mTexture.id > 0)
    {
        UnloadTexture(mTexture);
    }

    mParent = other.mParent;
    mPosition = other.mPosition;
    mMovement = other.mMovement;
    mVelocity = other.mVelocity;
    mAcceleration = other.mAcceleration;
    mScale = other.mScale;
    mTextureOffset = other.mTextureOffset;
    mColliderDimensions = other.mColliderDimensions;
    mTexture = other.mTexture;
    mOwnsTexture = other.mOwnsTexture;
    mTextureType = other.mTextureType;
    mAngle = other.mAngle;
    mSpriteSheetDimensions = other.mSpriteSheetDimensions;
    mAnimationIndices = std::move(other.mAnimationIndices);
    mFrameSpeed = other.mFrameSpeed;
    mCurrentFrameIndex = other.mCurrentFrameIndex;
    mAnimationTime = other.mAnimationTime;
    mIsJumping = other.mIsJumping;
    mJumpingPower = other.mJumpingPower;
    mIsActive = other.mIsActive;
    enableControl = other.enableControl;
    mAIActive = other.mAIActive;
    canCollide = other.canCollide;
    mIsTextureAtlas = other.mIsTextureAtlas;
    mIsHorizontalFlipped = other.mIsHorizontalFlipped;
    mIsVerticalFlipped = other.mIsVerticalFlipped;
    mSpeed = other.mSpeed;
    mIsCollidingTop = other.mIsCollidingTop;
    mIsCollidingBottom = other.mIsCollidingBottom;
    mIsCollidingRight = other.mIsCollidingRight;
    mIsCollidingLeft = other.mIsCollidingLeft;

    other.mParent = nullptr;
    other.mTexture = Texture2D{};
    other.mAnimationIndices.clear();
    other.mIsActive = false;
    other.mOwnsTexture = false;
    other.mTextureOffset = {0.0f, 0.0f};

    return *this;
}

Entity::~Entity()
{
    if (mOwnsTexture && mTexture.id > 0)
    {
        UnloadTexture(mTexture);
    }
}

bool Entity::isColliding(const Entity *other) const
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

bool Entity::isColliding(Vector2 position) const
{
    return fabs(mPosition.x - position.x) < (mColliderDimensions.x / 2.0f) &&
           fabs(mPosition.y - position.y) < (mColliderDimensions.y / 2.0f);
}

bool Entity::intersects(const Entity &other) const
{
    if (&other == this || !other.mIsActive) return false;

    return isColliding(&other);
}

void Entity::checkCollisionY(const std::vector<Entity*> &collidableEntities)
{
    for (Entity *entity : collidableEntities)
    {
        if (entity == this || !canCollide || !isColliding(entity)) continue;

        float yDistance = mPosition.y - entity->mPosition.y;
        float yOverlap = ((mColliderDimensions.y + entity->mColliderDimensions.y) / 2.0f) - fabs(yDistance);
        if (yOverlap <= 0.0f) continue;

        float xDistance = mPosition.x - entity->mPosition.x;
        float xOverlap = ((mColliderDimensions.x + entity->mColliderDimensions.x) / 2.0f) - fabs(xDistance);
        if (xOverlap <= 0.0f) continue;
        if (yOverlap > xOverlap) continue;

        float direction = 0.0f;
        if (yDistance > 0.0f) direction = 1.0f;
        else if (yDistance < 0.0f) direction = -1.0f;
        else
        {
            float relativeVelocity = mVelocity.y - entity->mVelocity.y;
            if (relativeVelocity > 0.0f) direction = 1.0f;
            else if (relativeVelocity < 0.0f) direction = -1.0f;
            else direction = 1.0f;
        }

        mPosition.y += direction * yOverlap;
        mVelocity.y  = 0;

        if (direction < 0.0f)
        {
            mIsCollidingBottom = true;
        }
        else
        {
            mIsCollidingTop = true;
        }
    }
}

void Entity::checkCollisionX(const std::vector<Entity*> &collidableEntities)
{
    for (Entity *entity : collidableEntities)
    {
        if (entity == this || !canCollide || !isColliding(entity)) continue;

        float xDistance = mPosition.x - entity->mPosition.x;
        float xOverlap = ((mColliderDimensions.x + entity->mColliderDimensions.x) / 2.0f) - fabs(xDistance);
        if (xOverlap <= 0.0f) continue;

        float yDistance = mPosition.y - entity->mPosition.y;
        float yOverlap = ((mColliderDimensions.y + entity->mColliderDimensions.y) / 2.0f) - fabs(yDistance);
        if (yOverlap <= 0.0f) continue;
        if (xOverlap > yOverlap) continue;

        float direction = 0.0f;
        if (xDistance > 0.0f) direction = 1.0f;
        else if (xDistance < 0.0f) direction = -1.0f;
        else
        {
            float relativeVelocity = mVelocity.x - entity->mVelocity.x;
            if (relativeVelocity > 0.0f) direction = 1.0f;
            else if (relativeVelocity < 0.0f) direction = -1.0f;
            else direction = 1.0f;
        }

        mPosition.x += direction * xOverlap;
        mVelocity.x  = 0;

        if (direction < 0.0f)
        {
            mIsCollidingRight = true;
        }
        else
        {
            mIsCollidingLeft = true;
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

void Entity::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    if (!mIsActive) return;

    if (mAIActive) AIupdate(deltaTime);

    resetColliderFlags();

    if (mMass > 0.0f) {mAcceleration += mForce * deltaTime / mMass; mForce = {0.0f, 0.0f};}
    mVelocity += mAcceleration * deltaTime;
    mPosition += mVelocity * deltaTime;
    mPosition += mMovement;
    mMovement = {0.0f, 0.0f};

    // resolve collisions
    checkCollisionY(collidableEntities);
    checkCollisionX(collidableEntities);
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

    if (mUseCustomSourceRect)
    {
        sourceArea = mCustomSourceRect;
    }

    if (mIsHorizontalFlipped)
    {
        sourceArea.width = -sourceArea.width;
    }

    if (mIsVerticalFlipped)
    {
        sourceArea.height = -sourceArea.height;
    }

    Rectangle destinationArea = {
        mPosition.x + mTextureOffset.x,
        mPosition.y + mTextureOffset.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 textureOrigin = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(mTexture, sourceArea, destinationArea, textureOrigin, mAngle, WHITE);
}

void Entity::shutdown()
{
    // Default implementation does nothing
    // Override this in derived classes for cleanup
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
