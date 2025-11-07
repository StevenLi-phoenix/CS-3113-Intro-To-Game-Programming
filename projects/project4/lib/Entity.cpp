#include "Entity.h"

#include <algorithm>
#include <cmath>

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {0}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT}, 
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} 
{
    mFlyAnchor = mPosition;
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f}, 
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}}, 
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
    mAngle {0.0f}, mEntityType {entityType} 
{
    mFlyAnchor = mPosition;
}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
        std::vector<int>> animationAtlas, EntityType entityType) : 
        mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} 
{
    mFlyAnchor = mPosition;
}

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        // STEP 1: For every entity that our player can collide with...
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {
            // STEP 2: Calculate the distance between its centre and our centre
            //         and use that to calculate the amount of overlap between
            //         both bodies.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                              (collidableEntity->mColliderDimensions.y / 2.0f));
            
            // STEP 3: "Unclip" ourselves from the other entity, and zero our
            //         vertical velocity.
            if (mVelocity.y > 0) 
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0) 
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {            
            // When standing on a platform, we're always slightly overlapping
            // it vertically due to gravity, which causes false horizontal
            // collision detections. So the solution I dound is only resolve X
            // collisions if there's significant Y overlap, preventing the 
            // platform we're standing on from acting like a wall.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            // Skip if barely touching vertically (standing on platform)
            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;

                // Collision!
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
 
                // Collision!
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

    // COLLISION ABOVE (jumping upward)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) && mVelocity.y < 0.0f)
    {
        mPosition.y += yOverlap;   // push down
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (falling downward)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap;   // push up
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

    // COLLISION ON RIGHT (moving right)
    if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;   // push left
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }

    // COLLISION ON LEFT (moving left)
    if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01;   // push right
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

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

bool Entity::intersects(const Entity &other) const
{
    if (&other == this || !other.isActive()) return false;

    float xDistance = fabs(mPosition.x - other.getPosition().x) -
        ((mColliderDimensions.x + other.getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other.getPosition().y) -
        ((mColliderDimensions.y + other.getColliderDimensions().y) / 2.0f);

    return (xDistance < 0.0f && yDistance < 0.0f);
}

void Entity::animate(float deltaTime)
{
    auto atlasIt = mAnimationAtlas.find(mDirection);
    if (atlasIt == mAnimationAtlas.end() || atlasIt->second.empty()) return;

    mAnimationIndices = atlasIt->second;
    if (mCurrentFrameIndex >= static_cast<int>(mAnimationIndices.size()))
        mCurrentFrameIndex = 0;

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::AIFollow(Entity *target)
{
    if (!target)
    {
        resetMovement();
        return;
    }

    const float distanceToTarget = Vector2Distance(mPosition, target->getPosition());
    if (distanceToTarget <= mFollowRadius) mAIState = FOLLOWING;
    else if (distanceToTarget > mFollowRadius * 1.2f) mAIState = WALKING;

    resetMovement();

    if (mAIState == FOLLOWING)
    {
        const float dx = target->getPosition().x - mPosition.x;
        if (fabsf(dx) > mFollowStopRadius)
        {
            if (dx > 0.0f) moveRight();
            else          moveLeft();
        }

        // Encourage jumps when the player is horizontally close but higher up.
        const float dy = target->getPosition().y - mPosition.y;
        if (dy < -mColliderDimensions.y * 0.5f && mIsCollidingBottom)
        {
            jump();
        }
    }
    else if (mHasPatrolBounds)
    {
        if (mPosition.x <= mPatrolLeft) mWanderDirection = 1;
        if (mPosition.x >= mPatrolRight) mWanderDirection = -1;

        if (mWanderDirection < 0) moveLeft();
        else                      moveRight();
    }
}

void Entity::AIWander()
{
    if (!mHasPatrolBounds)
    {
        if (mMovement.x == 0.0f) moveLeft();
        return;
    }

    const float margin = 4.0f;
    if (mPosition.x <= (mPatrolLeft + margin)) mWanderDirection = 1;
    if (mPosition.x >= (mPatrolRight - margin)) mWanderDirection = -1;

    if (mCurrentMap)
    {
        const float halfWidth = mColliderDimensions.x * 0.5f;
        const float heading = (mWanderDirection < 0) ? -1.0f : 1.0f;
        const float aheadOffset = halfWidth + std::max(6.0f, static_cast<float>(mSpeed) * 0.015f);
        const Vector2 probe = {
            mPosition.x + heading * aheadOffset,
            mPosition.y + (mColliderDimensions.y * 0.5f) + 4.0f
        };
        float xOverlap = 0.0f;
        float yOverlap = 0.0f;
        if (!mCurrentMap->isSolidTileAt(probe, &xOverlap, &yOverlap))
        {
            mWanderDirection *= -1;
        }
    }

    resetMovement();
    if (mWanderDirection < 0) moveLeft();
    else                      moveRight();
}

void Entity::AIFly(float deltaTime)
{
    if (!mFlyConfigured) return;

    if (!mFlyAnchorInitialised)
    {
        mFlyAnchor = mPosition;
        mFlyAnchorInitialised = true;
        mFlyTimer = 0.0f;
    }

    const float leftLimit = mFlyAnchor.x - mFlyHorizontalRange;
    const float rightLimit = mFlyAnchor.x + mFlyHorizontalRange;
    if (mPosition.x <= leftLimit) mFlyDirection = 1.0f;
    else if (mPosition.x >= rightLimit) mFlyDirection = -1.0f;

    resetMovement();
    if (mFlyHorizontalRange > 0.0f)
    {
        if (mFlyDirection < 0.0f) moveLeft();
        else                      moveRight();
    }

    mSpeed = static_cast<int>(mFlyHorizontalSpeed);
    mAcceleration.y = 0.0f;
    mVelocity.y = 0.0f;

    mFlyTimer += deltaTime;
    const float oscillation = sinf(mFlyTimer * mFlyVerticalFrequency * 6.2831853f) * mFlyVerticalAmplitude;
    mPosition.y = mFlyAnchor.y + oscillation;
}

void Entity::resetFlyAnchor()
{
    mFlyAnchorInitialised = false;
    mFlyTimer = 0.0f;
}

void Entity::clampToPatrolBounds()
{
    if (!mHasPatrolBounds) return;
    if (mPatrolLeft > mPatrolRight) return;

    if (mPosition.x < mPatrolLeft)      mPosition.x = mPatrolLeft;
    else if (mPosition.x > mPatrolRight) mPosition.x = mPatrolRight;
}

void Entity::AIActivate(Entity *target, float deltaTime)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander();
        break;

    case FOLLOWER:
        AIFollow(target);
        break;

    case FLYER:
        AIFly(deltaTime);
        break;
    
    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, 
    Entity *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;
    
    mCurrentMap = map;

    if (mEntityType == NPC) AIActivate(player, deltaTime);

    resetColliderFlags();

    mVelocity.x = mMovement.x * mSpeed;
    if (mMovement.y != 0.0f)
    {
        mVelocity.y = mMovement.y * mSpeed;
    }

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    // ––––– JUMPING ––––– //
    if (mIsJumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        mIsJumping = false;
        
        // STEP 2: The player now acquires an upward velocity
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);

    bool hasAtlas = mTextureType == ATLAS && !mAnimationAtlas.empty() && mFrameSpeed > 0;
    if (hasAtlas) animate(deltaTime);

    if (mEntityType == NPC)
    {
        clampToPatrolBounds();
    }
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
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

    // Destination rectangle – centred on gPosition
    Rectangle sourceArea = textureArea;

    if (mDirection == LEFT)
    {
        sourceArea.width = -sourceArea.width;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 renderOffset = getRenderOffset();
    destinationArea.x += renderOffset.x;
    destinationArea.y += renderOffset.y;

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(
        mTexture,
        sourceArea,
        destinationArea,
        originOffset,
        mAngle,
        WHITE
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
        colliderBox.x,      // Top-left X
        colliderBox.y,      // Top-left Y
        colliderBox.width,  // Width
        colliderBox.height, // Height
        GREEN               // Color
    );
}
