#ifndef ENTITY_H
#define ENTITY_H

#include "helper.h"

enum Direction    { DIRECTION_LEFT, DIRECTION_UP, DIRECTION_RIGHT, DIRECTION_DOWN               };
enum EntityStatus { ENTITY_STATUS_ACTIVE, ENTITY_STATUS_INACTIVE                              };
// enum EntityType   { ENTITY_PLAYER, ENTITY_BLOCK, ENTITY_PLATFORM, ENTITY_NPC, ENTITY_ANYMOUSE   };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mVelocity;
    Vector2 mAcceleration;
    Vector2 mPendingForce;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;
    bool mTextureAtlas; // false = single texture, true = texture atlas
    Vector2 mSpriteSheetDimensions;

    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;
    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    int mSpeed() { return GetLength(mVelocity); }
    float mAngle() { return getAngle(mVelocity); }

    bool mIsCollidingTop = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight = false;
    bool mIsCollidingLeft = false;

    EntityStatus mEntityStatus = ENTITY_STATUS_ACTIVE;




}
