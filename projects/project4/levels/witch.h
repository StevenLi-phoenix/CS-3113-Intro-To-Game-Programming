#ifndef WITCH_H
#define WITCH_H

#include "../lib/Entity.h"
#include <map>
#include <string>

enum WitchAnimation {
    WITCH_STATE_ATTACK,
    WITCH_STATE_BACK_START_RUN,
    WITCH_STATE_DEATH,
    WITCH_STATE_FALL,
    WITCH_STATE_HIT,
    WITCH_STATE_IDLE_ATTACK,
    WITCH_STATE_IDLE_BACK,
    WITCH_STATE_IDLE,
    WITCH_STATE_JUMP,
    WITCH_STATE_RUN,
    WITCH_STATE_START_RUN,
    WITCH_STATE_STOP_RUN,
    WITCH_STATE_TURN
};

struct AnimationInfo {
    int row = 0;
    int frameCount = 0;
};

struct VariantMetadata {
    int squareSize = 0;
    int rows = 0;
    int columns = 0;
    std::map<WitchAnimation, AnimationInfo> animations;
};

class Witch : public Entity
{
private:
    WitchAnimation mCurrentAnimation;
    std::string mVariant;
    VariantMetadata mMetadata;
    int mAtlasRows = 0;
    int mAtlasColumns = 0;
    bool mMovementRequested = false;
    bool mAnimationQueued = false;
    bool mIsAirborne = false;
    bool mHasGroundPlane = false;
    float mGroundPlaneY = 0.0f;
    float mControlLockTimer = 0.0f;
    bool mLandingAnimationPending = false;
    bool mIsTurning = false;
    Direction mFacingDirection = RIGHT;
    Direction mPendingFacingDirection = RIGHT;
    bool mHasPendingFacingDirection = false;
    bool mWasMoving = false;

    static constexpr float LANDING_LOCK_DURATION = 0.18f;
    static constexpr float TURN_LOCK_DURATION = 0.10f;
    static constexpr float TEXTURE_OFFSET_X = 0.0f;              // Manual world-space X tweak
    static constexpr float TEXTURE_OFFSET_Y_WORLD = 0.0f;        // Manual world-space Y tweak
    static constexpr float TEXTURE_BOTTOM_PADDING_PIXELS = 9.0f; // Empty pixels below the feet in atlas frames

    float mAtlasPixelScale = 1.0f;

    enum class ControlLockReason
    {
        None,
        Landing,
        Turn,
        StartRun
    };

    ControlLockReason mControlLockReason = ControlLockReason::None;

    AnimationInfo getAnimationInfo(WitchAnimation animation) const;
    bool hasAnimation(WitchAnimation animation) const;
    bool loadVariantMetadata(const std::string &variant, VariantMetadata &outMetadata) const;
    bool applyVariantMetadata(const std::string &variant, const VariantMetadata &metadata);
    void loadAnimation(WitchAnimation animation);
    void queueAnimation(WitchAnimation animation);
    bool shouldReturnToIdle() const;
    bool groundedByPlane() const;
    void alignToGroundPlane();
    void handleAirAndLanding(float deltaTime);
    void updateControlLock(float deltaTime);
    void lockControls(ControlLockReason reason, float duration);
    void applyPendingFacing();

public:
    Witch();
    explicit Witch(const std::string &variant);

    bool setVariant(const std::string &variant);

    void beginInputFrame();
    void finalizeInputFrame();

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    bool tryJump();
    void setGroundPlane(float y);
    bool isAirborne() const { return mIsAirborne; }
    bool controlsLocked() const { return mControlLockReason != ControlLockReason::None; }

    using Entity::update;
    Vector2 getRenderOffset() const override
    {
        const Vector2 scale = getScale();
        const Vector2 collider = getColliderDimensions();
        const float baseY = (collider.y - scale.y) * 0.5f;
        const float paddingY = mAtlasPixelScale * TEXTURE_BOTTOM_PADDING_PIXELS;
        return {TEXTURE_OFFSET_X, baseY + paddingY + TEXTURE_OFFSET_Y_WORLD};
    }
    void update(float deltaTime, Entity *player, Map *map, Entity *collidables, int count);
    void playAnimation(WitchAnimation animation);
    WitchAnimation getCurrentAnimation() const { return mCurrentAnimation; }
    const std::string &getVariant() const { return mVariant; }

    void playAttack()    { queueAnimation(WITCH_STATE_ATTACK);      }
    void playRun()       { queueAnimation(WITCH_STATE_RUN);         }
    void playIdle()      { queueAnimation(WITCH_STATE_IDLE);        }
    void playJump()      { queueAnimation(WITCH_STATE_JUMP);        }
    void playFall()      { queueAnimation(WITCH_STATE_FALL);        }
    void playDeath()     { queueAnimation(WITCH_STATE_DEATH);       }
    void playHit()       { queueAnimation(WITCH_STATE_HIT);         }
    void playTurn()      { queueAnimation(WITCH_STATE_TURN);        }
    void playStartRun()  { queueAnimation(WITCH_STATE_START_RUN);   }
    void playStopRun()   { queueAnimation(WITCH_STATE_STOP_RUN);    }
};

#endif
