#include "witch.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    const std::unordered_map<std::string, WitchAnimation> NAME_TO_ANIMATION = {
        {"attack", WITCH_STATE_ATTACK},
        {"atack", WITCH_STATE_ATTACK},  // legacy spelling
        {"back_start_run", WITCH_STATE_BACK_START_RUN},
        {"death", WITCH_STATE_DEATH},
        {"fall", WITCH_STATE_FALL},
        {"hit", WITCH_STATE_HIT},
        {"idle_attack", WITCH_STATE_IDLE_ATTACK},
        {"idle_atack", WITCH_STATE_IDLE_ATTACK}, // legacy spelling
        {"idle_back", WITCH_STATE_IDLE_BACK},
        {"idle", WITCH_STATE_IDLE},
        {"jump", WITCH_STATE_JUMP},
        {"run", WITCH_STATE_RUN},
        {"start_run", WITCH_STATE_START_RUN},
        {"stop_run", WITCH_STATE_STOP_RUN},
        {"turn", WITCH_STATE_TURN}
    };

    std::string metadataPathForVariant(const std::string &variant)
    {
        return "assets/witch/" + variant + "/witch_animations.json";
    }

    std::string atlasPathForVariant(const std::string &variant)
    {
        return "assets/witch/" + variant + "/animate.png";
    }
}

Witch::Witch()
    : Witch("Fire")
{
}

Witch::Witch(const std::string &variant)
    : Entity(),
      mCurrentAnimation(WITCH_STATE_IDLE),
      mVariant("Fire")
{
    setPosition(Vector2{600.0f, 337.5f});
    setScale(Vector2{100.0f, 100.0f});
    setSpeed(300);
    setFrameSpeed(DEFAULT_FRAME_SPEED);
    setJumpingPower(500.0f);
    setAcceleration(Vector2{0.0f, 900.0f});
    setColliderDimensions(Vector2{80.0f, 90.0f});
    setEntityType(PLAYER);

    if (!setVariant(variant))
    {
        TraceLog(LOG_ERROR, "Witch: failed to load variant '%s', attempting fallback to 'Fire'", variant.c_str());
        if (variant != "Fire" && !setVariant("Fire"))
        {
            TraceLog(LOG_FATAL, "Witch: failed to load fallback variant 'Fire'");
        }
    }
}

bool Witch::setVariant(const std::string &variant)
{
    if (variant == mVariant && hasAnimation(mCurrentAnimation))
    {
        return true;
    }

    VariantMetadata metadata;
    if (!loadVariantMetadata(variant, metadata))
    {
        return false;
    }

    return applyVariantMetadata(variant, metadata);
}

bool Witch::loadVariantMetadata(const std::string &variant, VariantMetadata &outMetadata) const
{
    const std::string path = metadataPathForVariant(variant);
    std::ifstream file(path);
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "Witch: unable to open metadata file '%s'", path.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    VariantMetadata metadata;

    {
        const std::regex squareRegex(R"regex("square_size"\s*:\s*([0-9]+))regex");
        std::smatch match;
        if (std::regex_search(content, match, squareRegex))
        {
            metadata.squareSize = std::stoi(match[1].str());
        }
        else
        {
            TraceLog(LOG_ERROR, "Witch: missing 'square_size' in metadata '%s'", path.c_str());
            return false;
        }
    }

    {
        const std::regex gridRegex(R"regex("grid"\s*:\s*\{\s*"columns"\s*:\s*([0-9]+)\s*,\s*"rows"\s*:\s*([0-9]+))regex");
        std::smatch match;
        if (std::regex_search(content, match, gridRegex))
        {
            metadata.columns = std::stoi(match[1].str());
            metadata.rows = std::stoi(match[2].str());
        }
        else
        {
            TraceLog(LOG_ERROR, "Witch: missing 'grid' definition in metadata '%s'", path.c_str());
            return false;
        }
    }

    metadata.animations.clear();
    const std::regex animationRegex(R"regex("([A-Za-z0-9_]+)"\s*:\s*\{\s*"row"\s*:\s*([0-9]+)\s*,\s*"frame_count"\s*:\s*([0-9]+))regex");

    for (std::sregex_iterator it(content.begin(), content.end(), animationRegex);
         it != std::sregex_iterator{};
         ++it)
    {
        const std::string name = (*it)[1].str();
        const int row = std::stoi((*it)[2].str());
        const int frameCount = std::stoi((*it)[3].str());

        auto animIt = NAME_TO_ANIMATION.find(name);
        if (animIt == NAME_TO_ANIMATION.end())
        {
            TraceLog(LOG_WARNING, "Witch: unknown animation '%s' in '%s'", name.c_str(), path.c_str());
            continue;
        }

        AnimationInfo info;
        info.row = row;
        info.frameCount = frameCount;
        metadata.animations[animIt->second] = info;
    }

    if (metadata.animations.empty())
    {
        TraceLog(LOG_ERROR, "Witch: no recognised animations found in '%s'", path.c_str());
        return false;
    }

    outMetadata = metadata;
    return true;
}

bool Witch::applyVariantMetadata(const std::string &variant, const VariantMetadata &metadata)
{
    if (metadata.columns <= 0 || metadata.rows <= 0)
    {
        TraceLog(LOG_ERROR, "Witch: invalid atlas dimensions for variant '%s'", variant.c_str());
        return false;
    }

    const std::string atlasPath = atlasPathForVariant(variant);
    setTexture(atlasPath.c_str());
    setTextureType(ATLAS);
    setSpriteSheetDimensions(Vector2{
        static_cast<float>(metadata.rows),
        static_cast<float>(metadata.columns)});

    float baseSize = static_cast<float>(metadata.squareSize);
    if (baseSize <= 0.0f)
    {
        baseSize = 48.0f;
    }

    const float renderScale = baseSize * 2.0f;
    setScale(Vector2{renderScale, renderScale});
    setColliderDimensions(Vector2{renderScale * 0.55f, renderScale * 0.9f});
    mAtlasPixelScale = (baseSize > 0.0f) ? (renderScale / baseSize) : 1.0f;

    mVariant = variant;
    mMetadata = metadata;
    mAtlasRows = metadata.rows;
    mAtlasColumns = metadata.columns;

    if (!hasAnimation(mCurrentAnimation))
    {
        if (hasAnimation(WITCH_STATE_IDLE))
        {
            mCurrentAnimation = WITCH_STATE_IDLE;
        }
        else
        {
            mCurrentAnimation = mMetadata.animations.begin()->first;
        }
    }

    loadAnimation(mCurrentAnimation);
    return true;
}

AnimationInfo Witch::getAnimationInfo(WitchAnimation animation) const
{
    auto it = mMetadata.animations.find(animation);
    if (it == mMetadata.animations.end())
    {
        return {};
    }
    return it->second;
}

bool Witch::hasAnimation(WitchAnimation animation) const
{
    return mMetadata.animations.find(animation) != mMetadata.animations.end();
}

void Witch::queueAnimation(WitchAnimation animation)
{
    if (!hasAnimation(animation))
    {
        playAnimation(animation);
        return;
    }

    mAnimationQueued = true;
    playAnimation(animation);
}

void Witch::loadAnimation(WitchAnimation animation)
{
    if (!hasAnimation(animation))
    {
        TraceLog(LOG_WARNING, "Witch: animation %d not available for variant '%s'",
                 static_cast<int>(animation), mVariant.c_str());
        return;
    }

    if (mAtlasColumns <= 0)
    {
        TraceLog(LOG_ERROR, "Witch: cannot load animation without valid atlas columns");
        return;
    }

    const AnimationInfo animInfo = getAnimationInfo(animation);
    std::vector<int> frameIndices;
    frameIndices.reserve(animInfo.frameCount);

    const int baseIndex = animInfo.row * mAtlasColumns;
    for (int i = 0; i < animInfo.frameCount; ++i)
    {
        frameIndices.push_back(baseIndex + i);
    }

    std::map<Direction, std::vector<int>> animAtlas;
    animAtlas[LEFT] = frameIndices;
    animAtlas[RIGHT] = frameIndices;
    animAtlas[UP] = frameIndices;
    animAtlas[DOWN] = frameIndices;

    setAnimationAtlas(animAtlas);
    setDirection(mFacingDirection);
}

void Witch::playAnimation(WitchAnimation animation)
{
    if (mCurrentAnimation == animation)
    {
        return;
    }

    if (!hasAnimation(animation))
    {
        TraceLog(LOG_WARNING, "Witch: variant '%s' missing requested animation %d",
                 mVariant.c_str(), static_cast<int>(animation));
        return;
    }

    mCurrentAnimation = animation;
    loadAnimation(animation);
}

void Witch::beginInputFrame()
{
    resetMovement();
    mMovementRequested = false;
    mAnimationQueued = false;
    if (controlsLocked())
    {
        return;
    }
}

void Witch::moveLeft()
{
    if (controlsLocked()) return;

    if (!mIsAirborne && mFacingDirection == RIGHT && hasAnimation(WITCH_STATE_TURN))
    {
        mPendingFacingDirection = LEFT;
        mHasPendingFacingDirection = true;
        lockControls(ControlLockReason::Turn, TURN_LOCK_DURATION);
        queueAnimation(WITCH_STATE_TURN);
        return;
    }

    mFacingDirection = LEFT;
    Entity::moveLeft();
    mMovementRequested = true;
}

void Witch::moveRight()
{
    if (controlsLocked()) return;

    if (!mIsAirborne && mFacingDirection == LEFT && hasAnimation(WITCH_STATE_TURN))
    {
        mPendingFacingDirection = RIGHT;
        mHasPendingFacingDirection = true;
        lockControls(ControlLockReason::Turn, TURN_LOCK_DURATION);
        queueAnimation(WITCH_STATE_TURN);
        return;
    }

    mFacingDirection = RIGHT;
    Entity::moveRight();
    mMovementRequested = true;
}

void Witch::moveUp()
{
    if (controlsLocked()) return;
    Entity::moveUp();
    mMovementRequested = true;
}

void Witch::moveDown()
{
    if (controlsLocked()) return;
    Entity::moveDown();
    mMovementRequested = true;
}

bool Witch::shouldReturnToIdle() const
{
    switch (mCurrentAnimation)
    {
        case WITCH_STATE_RUN:
        case WITCH_STATE_START_RUN:
        case WITCH_STATE_STOP_RUN:
        case WITCH_STATE_BACK_START_RUN:
        case WITCH_STATE_IDLE_BACK:
        case WITCH_STATE_IDLE_ATTACK:
        case WITCH_STATE_IDLE:
        case WITCH_STATE_TURN:
        case WITCH_STATE_FALL:
            return true;
        default:
            return false;
    }
}

void Witch::finalizeInputFrame()
{
    if (controlsLocked())
    {
        if (mControlLockReason == ControlLockReason::Landing && mLandingAnimationPending)
        {
            playAnimation(WITCH_STATE_FALL);
            mLandingAnimationPending = false;
            mAnimationQueued = true;
        }
        return;
    }

    bool wantsMovement = mMovementRequested;

    if (!mAnimationQueued)
    {
        if (wantsMovement)
        {
            normaliseMovement();
            if (!mWasMoving && hasAnimation(WITCH_STATE_START_RUN))
            {
                playAnimation(WITCH_STATE_START_RUN);
                const AnimationInfo info = getAnimationInfo(WITCH_STATE_START_RUN);
                float duration = info.frameCount > 0
                    ? static_cast<float>(info.frameCount) / static_cast<float>(getFrameSpeed())
                    : TURN_LOCK_DURATION;
                lockControls(ControlLockReason::StartRun, duration);
                mWasMoving = true;
                return;
            }
            playAnimation(WITCH_STATE_RUN);
            mWasMoving = true;
        }
        else if (shouldReturnToIdle())
        {
            playAnimation(WITCH_STATE_IDLE);
            mWasMoving = false;
        }
        else
        {
            mWasMoving = false;
        }
    }
    else
    {
        if (!wantsMovement)
        {
            mWasMoving = false;
        }
        else
        {
            mWasMoving = true;
        }
    }
}

void Witch::tryJump()
{
    if (controlsLocked() || mIsAirborne)
    {
        return;
    }

    jump();
    mIsAirborne = true;
    playAnimation(WITCH_STATE_JUMP);
}

void Witch::setGroundPlane(float y)
{
    mHasGroundPlane = true;
    mGroundPlaneY = y;
    Vector2 position = getPosition();
    position.y = y;
    setPosition(position);
}

bool Witch::groundedByPlane() const
{
    if (!mHasGroundPlane) return false;
    return getPosition().y >= mGroundPlaneY - 0.5f;
}

void Witch::alignToGroundPlane()
{
    if (!mHasGroundPlane) return;

    Vector2 position = getPosition();
    if (position.y > mGroundPlaneY)
    {
        position.y = mGroundPlaneY;
        setPosition(position);
    }

    Vector2 velocity = getVelocity();
    if (velocity.y > 0.0f)
    {
        velocity.y = 0.0f;
        setVelocity(velocity);
    }
}

void Witch::handleAirAndLanding(float deltaTime)
{
    (void)deltaTime;
    bool grounded = isCollidingBottom();
    if (!grounded && groundedByPlane())
    {
        alignToGroundPlane();
        grounded = true;
    }

    if (grounded)
    {
        if (mIsAirborne)
        {
            mIsAirborne = false;
            mLandingAnimationPending = true;
            lockControls(ControlLockReason::Landing, LANDING_LOCK_DURATION);
            mWasMoving = false;
        }
    }
    else
    {
        if (!mIsAirborne)
        {
            mIsAirborne = true;
        }

        if (mCurrentAnimation != WITCH_STATE_JUMP &&
            mCurrentAnimation != WITCH_STATE_ATTACK &&
            mCurrentAnimation != WITCH_STATE_HIT &&
            mCurrentAnimation != WITCH_STATE_DEATH)
        {
            playAnimation(WITCH_STATE_JUMP);
        }
    }
}

void Witch::updateControlLock(float deltaTime)
{
    if (mControlLockReason == ControlLockReason::None)
    {
        return;
    }

    if (mControlLockTimer > 0.0f)
    {
        mControlLockTimer -= deltaTime;
    }

    if (mControlLockTimer > 0.0f)
    {
        return;
    }

    switch (mControlLockReason)
    {
        case ControlLockReason::Landing:
            mLandingAnimationPending = false;
            break;
        case ControlLockReason::Turn:
            mIsTurning = false;
            applyPendingFacing();
            break;
        case ControlLockReason::StartRun:
            playAnimation(WITCH_STATE_RUN);
            mWasMoving = true;
            break;
        case ControlLockReason::None:
            break;
    }

    mControlLockReason = ControlLockReason::None;
    mControlLockTimer = 0.0f;
}

void Witch::lockControls(ControlLockReason reason, float duration)
{
    mControlLockReason = reason;
    mControlLockTimer = duration > 0.0f ? duration : 0.0f;
    if (reason == ControlLockReason::Turn)
    {
        mIsTurning = true;
    }
}

void Witch::applyPendingFacing()
{
    if (!mHasPendingFacingDirection)
    {
        setDirection(mFacingDirection);
        return;
    }

    mFacingDirection = mPendingFacingDirection;
    setDirection(mFacingDirection);
    mHasPendingFacingDirection = false;
}

void Witch::update(float deltaTime, Entity *player, Map *map,
                   Entity *collidables, int count)
{
    Entity::update(deltaTime, player, map, collidables, count);
    handleAirAndLanding(deltaTime);
    updateControlLock(deltaTime);
}
