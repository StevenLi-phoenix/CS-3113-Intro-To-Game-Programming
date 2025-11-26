#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include "Helper.h"
#include "Map.h"

namespace EntityConstants {
    constexpr float DEFAULT_SIZE = 16.0f;
    constexpr float DEFAULT_SPEED = 100.0f;
    constexpr float DEFAULT_FRAME_SPEED = 10.0f;
    constexpr float DEFAULT_JUMP_POWER = 100.0f;
    constexpr float DEFAULT_GRAVITY = 10.0f;
    constexpr float DEFAULT_FRICTION = 0.9f;
    constexpr float DEFAULT_BOUNCE = 0.8f;
    enum TextureType { SINGLE, ATLAS };
}

class Entity
{
private:
    Entity *mParent = nullptr;
    
    // does not implemnted here, just a placeholder for future implementation
    int zIndex = 5; // 0 is the lowest, 10 is the highest (not enforced) // used for rendering order
    
    Vector2 mMovement; // instant movement
    Vector2 mPosition; // position of the entity
    Vector2 mVelocity; // velocity of the entity
    Vector2 mAcceleration; // acceleration of the entity
    Vector2 mForce; // force applied to the entity, pending force, will be applied in the update function

    float mMass = 0.0f; // default disabled for forced-based movement
    
    Vector2 mScale;
    Vector2 mTextureOffset;
    Vector2 mColliderDimensions;
    Color mTint{255, 255, 255, 255};

    Texture2D mTexture;
    bool mOwnsTexture = true;
    EntityConstants::TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;
    
    std::vector<int> mAnimationIndices;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    bool mIsJumping = false;
    float mJumpingPower = 0.0f;
    bool mIsActive = false; // True if the entity is active, false if it is inactive
    bool enableControl = false; // True if the entity is controllable by the player, false if it is not
    bool mAIActive = false; // If script AIupdate is called, this will be true
    bool canCollide = true; // True if the entity can collide with other entities, false if it cannot
    bool mIsPushable = false;
    bool mIsTextureAtlas = false; // True if the entity has a texture atlas, false if it does not
    bool mIsHorizontalFlipped = false; // True if the entity is horizontally flipped, false if it is not
    bool mIsVerticalFlipped = false; // True if the entity is vertically flipped, false if it is not
    bool mTextureFacesLeft = false; // True if the source texture faces left by default
    bool mUseCustomSourceRect = false;
    Rectangle mCustomSourceRect = {0, 0, 0, 0};

    float mSpeed;
    float mAngle;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    bool isColliding(const Entity *other) const;
    bool isColliding(Map *map) const;
    bool isColliding(Vector2 position) const; // for point detection

    void checkCollisionY(const std::vector<Entity*> &collidableEntities);
    void checkCollisionY(Map *map);

    void checkCollisionX(const std::vector<Entity*> &collidableEntities);
    void checkCollisionX(Map *map);

    void applyPushForces(const std::vector<Entity*> &collidableEntities, double *outMs = nullptr, size_t *outPairs = nullptr);
    void animate(float deltaTime);
    virtual void AIupdate(float deltaTime);
    void logPushSummaryIfNeeded();
    static inline double sPushTimeMs = 0.0;
    static inline size_t sPushPairs = 0;
    struct PerfBuckets
    {
        double aiMs;
        double moveMs;
        double pushMs;
        size_t pushPairs;
        double collideEntityMs;
        double collideMapMs;
        double lastLog;

        PerfBuckets();
    };
    static inline PerfBuckets sPerf;
public:
    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityConstants::TextureType textureType, Vector2 spriteSheetDimensions, std::vector<int> animationIndices);
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;
    ~Entity();

    virtual void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {});
    virtual void render();
    virtual void shutdown();
    void normaliseMovement() { Normalise(&mMovement); }
    bool intersects(const Entity &other) const;
    void displayCollider();

    // setters and getters
    void setZIndex(int value) { zIndex = value; }
    void setPosition(Vector2 position) { mPosition = position; }
    void setMovement(Vector2 movement) { mMovement = movement; }
    void setVelocity(Vector2 velocity) { mVelocity = velocity; }
    void setForce(Vector2 force) { mForce = force; }
    void addForce(Vector2 force) { mForce += force; }
    void setAcceleration(Vector2 acceleration) { mAcceleration = acceleration; }
    void setScale(Vector2 scale) { mScale = scale; }
    void setTextureOffset(Vector2 textureOffset) { mTextureOffset = textureOffset; }
    void setColliderDimensions(Vector2 colliderDimensions) { mColliderDimensions = colliderDimensions; }
    void setTexture(Texture2D texture) { mTexture = texture; }
    void setOwnsTexture(bool ownsTexture) { mOwnsTexture = ownsTexture; }
    void setParent(Entity *parent) { mParent = parent; }
    void setTextureType(EntityConstants::TextureType textureType) { mTextureType = textureType; }
    void setSpriteSheetDimensions(Vector2 spriteSheetDimensions) { mSpriteSheetDimensions = spriteSheetDimensions; }
    void setAnimationIndices(std::vector<int> animationIndices) { mAnimationIndices = animationIndices; }
    void setFrameSpeed(int frameSpeed) { mFrameSpeed = frameSpeed; }
    void setCurrentFrameIndex(int currentFrameIndex) { mCurrentFrameIndex = currentFrameIndex; }
    void setAnimationTime(float animationTime) { mAnimationTime = animationTime; }
    void setIsJumping(bool isJumping) { mIsJumping = isJumping; }
    void setJumpingPower(float jumpingPower) { mJumpingPower = jumpingPower; }
    void setIsActive(bool isActive) { mIsActive = isActive; }
    void setEnableControl(bool enableControl) { enableControl = enableControl; }
    void setAIActive(bool mAIActive) { this->mAIActive = mAIActive; }
    void setCanCollide(bool value) { canCollide = value; }
    void setIsPushable(bool value) { mIsPushable = value; }
    void setIsTextureAtlas(bool isTextureAtlas) { mIsTextureAtlas = isTextureAtlas; }
    void setIsHorizontalFlipped(bool isHorizontalFlipped) { mIsHorizontalFlipped = isHorizontalFlipped; }
    void setIsVerticalFlipped(bool isVerticalFlipped) { mIsVerticalFlipped = isVerticalFlipped; }
    void setTextureFacesLeft(bool facesLeft) { mTextureFacesLeft = facesLeft; }
    void setCustomSourceRect(const Rectangle &rect) { mCustomSourceRect = rect; mUseCustomSourceRect = true; }
    void clearCustomSourceRect() { mUseCustomSourceRect = false; mCustomSourceRect = {0, 0, 0, 0}; }
    void setTint(Color tint) { mTint = tint; }
    void setSpeed(float speed) { mSpeed = speed; }
    void setAngle(float angle) { mAngle = angle; }
    void setIsCollidingTop(bool isCollidingTop) { mIsCollidingTop = isCollidingTop; }
    void setIsCollidingBottom(bool isCollidingBottom) { mIsCollidingBottom = isCollidingBottom; }
    void setIsCollidingRight(bool isCollidingRight) { mIsCollidingRight = isCollidingRight; }
    void setIsCollidingLeft(bool isCollidingLeft) { mIsCollidingLeft = isCollidingLeft; }
    void resetColliderFlags() { mIsCollidingTop = false; mIsCollidingBottom = false; mIsCollidingRight = false; mIsCollidingLeft = false; }
    
    // getters
    int getZIndex() const { return zIndex; }
    Vector2 getPosition() const { return mPosition; }
    Entity* getParent() const { return mParent; }
    Vector2 getMovement() const { return mMovement; }
    Vector2 getVelocity() const { return mVelocity; }
    Vector2 getAcceleration() const { return mAcceleration; }
    Vector2 getForce() const { return mForce; }
    Vector2 getScale() const { return mScale; }
    Vector2 getTextureOffset() const { return mTextureOffset; }
    Vector2 getColliderDimensions() const { return mColliderDimensions; }
    Texture2D getTexture() const { return mTexture; }
    bool ownsTexture() const { return mOwnsTexture; }
    EntityConstants::TextureType getTextureType() const { return mTextureType; }
    Vector2 getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    std::vector<int> getAnimationIndices() const { return mAnimationIndices; }
    int getFrameSpeed() const { return mFrameSpeed; }
    int getCurrentFrameIndex() const { return mCurrentFrameIndex; }
    float getAnimationTime() const { return mAnimationTime; }
    bool getIsJumping() const { return mIsJumping; }
    float getJumpingPower() const { return mJumpingPower; }
    bool getIsActive() const { return mIsActive; }
    bool getEnableControl() const { return enableControl; }
    bool getAIActive() const { return mAIActive; }
    bool getCanCollide() const { return canCollide; }
    bool getIsPushable() const { return mIsPushable; }
    bool getIsTextureAtlas() const { return mIsTextureAtlas; }
    bool getIsHorizontalFlipped() const { return mIsHorizontalFlipped; }
    bool getTextureFacesLeft() const { return mTextureFacesLeft; }
    bool usesCustomSourceRect() const { return mUseCustomSourceRect; }
    Rectangle getCustomSourceRect() const { return mCustomSourceRect; }
    Color getTint() const { return mTint; }
};

#endif // ENTITY_H
