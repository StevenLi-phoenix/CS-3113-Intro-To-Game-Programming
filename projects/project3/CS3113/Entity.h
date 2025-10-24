#ifndef ENTITY_H
#define ENTITY_H

#include "cs3113.h"
#include "../lib/vector_ops.h"

enum Direction    { LEFT, UP, RIGHT, DOWN              }; // For walking
enum EntityStatus { ENTITY_ACTIVE, ENTITY_INACTIVE                   };
enum EntityType   { ENTITY_PLAYER, ENTITY_BLOCK, ENTITY_PLATFORM, ENTITY_NPC, ENTITY_ANYMOUSE };

class Entity
{
private:
    Entity* mParentEntity = nullptr;
    Vector2 mParentOffset {0.0f, 0.0f};

    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;
    Vector2 mPendingForce {0.0f, 0.0f};

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

    bool mIsJumping = false;
    float mJumpingPower = 0.0f;

    int mSpeed;
    float mAngle;
    float mAngularVelocity = 0.0f;
    float mAngularAcceleration = 0.0f;
    float mPendingTorque = 0.0f;
    float mMass = 1.0f;
    float mMomentOfInertia = 1.0f;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    bool mIsAIActive = false;

    EntityStatus mEntityStatus = ENTITY_ACTIVE;
    EntityType   mEntityType;

    bool isColliding(Entity *other) const;
    void checkCollisionY(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Entity *collidableEntities, int collisionCheckCount);
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 200;
    static constexpr int   DEFAULT_FRAME_SPEED   = 14;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;
    static constexpr float DEFAULT_FORCE         = 200.0f;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        EntityType entityType);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, 
        EntityType entityType);
    ~Entity();

    virtual void updateAI(float deltaTime){};

    virtual void update(float deltaTime);
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    void jump()       { mIsJumping = true;  }
    void activate()   { mEntityStatus  = ENTITY_ACTIVE;   }
    void deactivate() { mEntityStatus  = ENTITY_INACTIVE; }
    void displayCollider();

    bool isActive() { return mEntityStatus == ENTITY_ACTIVE ? true : false; }
    bool isAIActive() { return mIsAIActive; }

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mDirection = LEFT;  applyForce({-DEFAULT_FORCE, 0.0f}); }
    void moveRight() { mDirection = RIGHT; applyForce({ DEFAULT_FORCE, 0.0f}); }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mScale;                 }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    float       getJumpingPower()          const { return mJumpingPower;          }
    bool        isJumping()                const { return mIsJumping;             }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    float       getAngularVelocity()       const { return mAngularVelocity;       }
    float       getAngularAcceleration()   const { return mAngularAcceleration;   }
    float       getMass()                  const { return mMass;                  }
    float       getMomentOfInertia()       const { return mMomentOfInertia;       }
    EntityType  getEntityType()            const { return mEntityType;            }
    
    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }

    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    void setPosition(Vector2 newPosition)
    { 
        mPosition = newPosition;

        if (mParentEntity)
        {
            mParentOffset = mPosition - mParentEntity->getPosition();
        }
        else
        {
            mParentOffset = Vector2{0.0f, 0.0f};
        }
    }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;                 }
    void setVelocity(Vector2 newVelocity)
        { mVelocity = newVelocity;                 }
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
    void setJumpingPower(float newJumpingPower)
        { mJumpingPower = newJumpingPower;         }
    void setAngle(float newAngle) 
        { mAngle = newAngle;                       }
    void setAngularVelocity(float newAngularVelocity)
        { mAngularVelocity = newAngularVelocity;   }
    void setAngularAcceleration(float newAngularAcceleration)
        { mAngularAcceleration = newAngularAcceleration; }
    void setMass(float newMass)
        { mMass = (newMass <= 0.0f) ? 1.0f : newMass; }
    void setMomentOfInertia(float newMomentOfInertia)
        { mMomentOfInertia = (newMomentOfInertia <= 0.0f) ? 1.0f : newMomentOfInertia; }
    void setEntityType(EntityType entityType)
        { mEntityType = entityType;                }
    void setDirection(Direction newDirection)
    { 
        mDirection = newDirection;
        if (mTextureType == ATLAS) mAnimationIndices = mAnimationAtlas.at(mDirection);
    }
    void setAIActive(bool isActive)
        { mIsAIActive = isActive;                }
    void setParentEntity(Entity* parent);
    Entity* getParentEntity() const { return mParentEntity; }
   Vector2 getParentOffset() const { return mParentOffset; }
    void applyForce(Vector2 force);
    void applyTorque(float torque);
};

#endif // ENTITY_CPP
