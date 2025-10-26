#include "Entity.h"
#include <cmath>

namespace
{
Vector2 rotateVectorDeg(const Vector2& v, float degrees)
{
    float radians = degrees * DEG2RAD;
    float cosTheta = std::cos(radians);
    float sinTheta = std::sin(radians);
    return Vector2{
        v.x * cosTheta - v.y * sinTheta,
        v.x * sinTheta + v.y * cosTheta
    };
}
}

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mPendingForce {0.0f, 0.0f}, mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {}, mTextureType {SINGLE}, mSpriteSheetDimensions {},
                   mAnimationAtlas {{}}, mAnimationIndices {}, mDirection {RIGHT}, 
                   mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, mAngle {0.0f},
                   mAngularVelocity {0.0f}, mAngularAcceleration {0.0f},
                   mPendingTorque {0.0f}, mMass {1.0f}, mMomentOfInertia {1.0f},
                   mEntityType {ENTITY_ANYMOUSE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f}, 
    mAcceleration {0.0f, 0.0f}, mPendingForce {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
    mTextureType {SINGLE}, mSpriteSheetDimensions {}, mAnimationAtlas {{}}, 
    mAnimationIndices {}, mDirection {RIGHT}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
    mAngle {0.0f}, mAngularVelocity {0.0f}, mAngularAcceleration {0.0f},
    mPendingTorque {0.0f}, mMass {1.0f}, mMomentOfInertia {1.0f},
    mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
        std::vector<int>> animationAtlas, EntityType entityType) : 
        mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mPendingForce {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mSpeed { DEFAULT_SPEED }, mAngle { 0.0f }, 
        mAngularVelocity {0.0f}, mAngularAcceleration {0.0f},
        mPendingTorque {0.0f}, mMass {1.0f}, mMomentOfInertia {1.0f},
        mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::setParentEntity(Entity* parent)
{
    mParentEntity = parent;

    if (mParentEntity)
    {
        mParentOffset = mPosition - mParentEntity->getPosition();
        mParentLocalOffset = rotateVectorDeg(mParentOffset, -mParentEntity->getAngle());
        mHasParentLocalOffset = false;
        mInheritParentRotation = false;
        mParentAngleOffset = 0.0f;
    }
    else
    {
        mParentOffset = Vector2{0.0f, 0.0f};
        mParentLocalOffset = Vector2{0.0f, 0.0f};
        mHasParentLocalOffset = false;
        mInheritParentRotation = false;
        mParentAngleOffset = 0.0f;
    }
}

void Entity::setParentLocalOffset(Vector2 localOffset)
{
    mParentLocalOffset = localOffset;
    mHasParentLocalOffset = true;
    mParentOffset = {0.0f, 0.0f};
}

void Entity::setParentRotationInheritance(bool inheritRotation, float angleOffset)
{
    mInheritParentRotation = inheritRotation;
    mParentAngleOffset = inheritRotation ? angleOffset : 0.0f;
}

void Entity::applyForce(Vector2 force)
{
    mPendingForce = mPendingForce + force;
}

void Entity::applyTorque(float torque)
{
    mPendingTorque += torque;
}

/**
 * Iterates through a list of collidable entities, checks for collisions with
 * the player entity, and resolves any vertical overlap by adjusting the 
 * player's position and velocity accordingly.
 * 
 * @param collidableEntities An array of pointers to `Entity` objects that 
 * represent the entities that the current `Entity` instance can potentially
 * collide with. The `collisionCheckCount` parameter specifies the number of
 * entities in the `collidableEntities` array that need to be checked for
 * collision.
 * @param collisionCheckCount The number of entities that the current entity
 * (`Entity`) should check for collisions with. This parameter specifies how
 * many entities are in the `collidableEntities` array that need to be checked
 * for collisions with the current entity.
 */
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

                if (collidableEntity->mEntityType == ENTITY_BLOCK)
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

/**
 * Checks if two entities are colliding based on their positions and collider 
 * dimensions.
 * 
 * @param other represents another Entity with which you want to check for 
 * collision. It is a pointer to the Entity class.
 * 
 * @return returns `true` if the two entities are colliding based on their
 * positions and collider dimensions, and `false` otherwise.
 */
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

/**
 * Updates the current frame index of an entity's animation based on the 
 * elapsed time and frame speed.
 * 
 * @param deltaTime represents the time elapsed since the last frame update.
 */
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



void Entity::update(float deltaTime)
{
    // if (mEntityStatus == INACTIVE) return;
    
    // if (mEntityType == NPC) AIActivate(player);

    resetColliderFlags();

    // mVelocity.x = mMovement.x * mSpeed;

    Vector2 totalAcceleration = mAcceleration + ((1.0f / mMass) * mPendingForce);
    mVelocity = mVelocity + (deltaTime * totalAcceleration);
    mPendingForce = {0.0f, 0.0f};

    float netAngularAcceleration = mAngularAcceleration + (mPendingTorque / mMomentOfInertia);
    mAngularVelocity = mAngularVelocity + (deltaTime * netAngularAcceleration);
    mAngle = mAngle + (deltaTime * mAngularVelocity);
    mPendingTorque = 0.0f;

    if (mAngle > 360.0f)
        mAngle -= 360.0f;
    else if (mAngle < -360.0f)
        mAngle += 360.0f;

    // // ––––– JUMPING ––––– //
    // if (mIsJumping)
    // {
    //     // STEP 1: Immediately return the flag to its original false state
    //     mIsJumping = false;
        
    //     // STEP 2: The player now acquires an upward velocity
    //     mVelocity.y -= mJumpingPower;
    // }

    Vector2 delta = deltaTime * mVelocity;

    if (mParentEntity)
    {
        mParentOffset = mParentOffset + delta;

        Vector2 derivedOffset = mParentOffset;
        if (mHasParentLocalOffset)
        {
            Vector2 rotatedLocal = rotateVectorDeg(mParentLocalOffset, mParentEntity->getAngle());
            derivedOffset = derivedOffset + rotatedLocal;
        }

        mPosition = mParentEntity->getPosition() + derivedOffset;

        if (mInheritParentRotation)
        {
            mAngle = mParentEntity->getAngle() + mParentAngleOffset;
        }
    }
    else
    {
        mPosition = mPosition + delta;
        // checkCollisionY(collidableEntities, collisionCheckCount);
        // checkCollisionY(blocks, blockCount);

        // checkCollisionX(collidableEntities, collisionCheckCount);
        // checkCollisionX(blocks, blockCount);
    }

    if (mTextureType == ATLAS && GetLength(mMovement) != 0 && mIsCollidingBottom) 
        animate(deltaTime);

    // Update AI if active
    if (isAIActive()) updateAI(deltaTime);
}

void Entity::render()
{
    if(mEntityStatus == ENTITY_INACTIVE) return;

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
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

    // displayCollider();
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
