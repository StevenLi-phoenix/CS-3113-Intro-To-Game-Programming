#include "music_note.h"

#include <algorithm>
#include <cmath>

namespace
{
    struct NoteMeta
    {
        MusicNote::Variant variant;
        const char *tag;
        int damage;
    };

    constexpr NoteMeta NOTE_META[] = {
        {MusicNote::Variant::Note1,  "MUSIC1",    1},
        {MusicNote::Variant::Note2,  "MUSIC2",    1},
        {MusicNote::Variant::Note3,  "MUSIC3",    1},
        {MusicNote::Variant::Star,   "MUSICSTAR", 2}
    };

    const NoteMeta* findMeta(MusicNote::Variant variant)
    {
        for (const NoteMeta &meta : NOTE_META)
        {
            if (meta.variant == variant)
            {
                return &meta;
            }
        }
        return &NOTE_META[0];
    }
}

MusicNote::MusicNote(Variant variant,
                     Entity *parent,
                     FollowConfig config,
                     OrbitConfig orbit)
    : Entity(),
      mVariant(variant),
      mConfig(config),
      mOrbitConfig(orbit)
{
    setIsActive(true);
    setCanCollide(false);
    setParent(parent);
    setColliderDimensions({0.0f, 0.0f});
    if (parent)
    {
        setPosition(parent->getPosition());
    }
    refreshSprite();
}

void MusicNote::setVariant(Variant variant)
{
    if (mVariant == variant)
    {
        return;
    }
    mVariant = variant;
    refreshSprite();
}

int MusicNote::getDamage() const
{
    return damageForVariant(mVariant);
}

int MusicNote::damageForVariant(Variant variant)
{
    return findMeta(variant)->damage;
}

const char *MusicNote::tagForVariant(Variant variant)
{
    return findMeta(variant)->tag;
}

void MusicNote::setOrbitSlot(size_t slotIndex, size_t slotCount)
{
    mOrbitSlotIndex = slotIndex;
    mOrbitSlotCount = std::max<size_t>(1u, slotCount);
}

void MusicNote::setOrbitSuppressed(bool suppressed)
{
    mOrbitSuppressed = suppressed;
}

bool MusicNote::isAvailableForAttack() const
{
    return mState == State::Orbit;
}

void MusicNote::launchAttack(const Vector2 &impactPoint, AttackAnimConfig animConfig)
{
    mAttackStart = getPosition();
    mAttackImpact = impactPoint;
    mAttackTimer = 0.0f;
    mAttackDuration = std::max(animConfig.travelTime, 0.01f);
    mAttackReturnDuration = std::max(animConfig.returnTime, 0.01f);
    mState = State::AttackOut;
}

void MusicNote::update(float deltaTime,
                       Entity *player,
                       Map *map,
                       const std::vector<Entity*> &collidableEntities)
{
    (void)player;
    (void)map;
    (void)collidableEntities;
    if (!getIsActive())
    {
        return;
    }

    switch (mState)
    {
        case State::Orbit:
            updateOrbit(deltaTime);
            break;
        case State::AttackOut:
            updateAttackOut(deltaTime);
            break;
        case State::AttackReturn:
            updateAttackReturn(deltaTime);
            break;
    }
}

void MusicNote::refreshSprite()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlasTexture = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    if (!atlasTexture)
    {
        LOG_WARNING("MusicNote: world atlas texture missing");
        return;
    }

    setTexture(*atlasTexture);
    setOwnsTexture(false);
    const char *tag = tagForVariant(mVariant);
    Rectangle spriteRect = rm.getSpriteRect(tag);
    if (spriteRect.width <= 0.0f || spriteRect.height <= 0.0f)
    {
        LOG_WARNING(TextFormat("MusicNote: sprite '%s' missing from atlas metadata", tag));
        return;
    }

    const float targetHeight = combat::NOTE_DESIRED_HEIGHT;
    const float scaleFactor = targetHeight / spriteRect.height;
    Vector2 spriteSize = {
        spriteRect.width * scaleFactor,
        targetHeight
    };
    setScale(spriteSize);
    setCustomSourceRect(spriteRect);
}

void MusicNote::updateOrbit(float deltaTime)
{
    if (mOrbitSuppressed)
    {
        updateFollowLag(deltaTime);
        return;
    }

    mOrbitAngle += mOrbitConfig.angularSpeed * deltaTime;
    if (mOrbitAngle > PI * 2.0f)
    {
        mOrbitAngle = std::fmod(mOrbitAngle, PI * 2.0f);
    }

    const Vector2 desired = computeOrbitTarget();
    Vector2 current = getPosition();
    const float lerpFactor = std::clamp(mConfig.lerpSpeed * deltaTime, 0.0f, 1.0f);
    Vector2 newPosition = {
        current.x + (desired.x - current.x) * lerpFactor,
        current.y + (desired.y - current.y) * lerpFactor
    };

    setPosition(applyBobOffset(newPosition, deltaTime));
}

void MusicNote::updateAttackOut(float deltaTime)
{
    mAttackTimer += deltaTime;
    const float duration = std::max(mAttackDuration, 0.0001f);
    const float t = std::clamp(mAttackTimer / duration, 0.0f, 1.0f);

    Vector2 newPosition = {
        mAttackStart.x + (mAttackImpact.x - mAttackStart.x) * t,
        mAttackStart.y + (mAttackImpact.y - mAttackStart.y) * t
    };

    setPosition(applyBobOffset(newPosition, deltaTime));

    if (t >= 1.0f)
    {
        mState = State::AttackReturn;
        mAttackTimer = 0.0f;
        mAttackStart = mAttackImpact;
    }
}

void MusicNote::updateAttackReturn(float deltaTime)
{
    mAttackTimer += deltaTime;
    const float duration = std::max(mAttackReturnDuration, 0.0001f);
    const float t = std::clamp(mAttackTimer / duration, 0.0f, 1.0f);

    Vector2 targetPosition = mOrbitSuppressed ? computeLaggedCenter() : computeOrbitTarget();
    Vector2 newPosition = {
        mAttackStart.x + (targetPosition.x - mAttackStart.x) * t,
        mAttackStart.y + (targetPosition.y - mAttackStart.y) * t
    };

    setPosition(applyBobOffset(newPosition, deltaTime));

    if (t >= 1.0f)
    {
        mState = State::Orbit;
        mAttackTimer = 0.0f;
    }
}

void MusicNote::updateFollowLag(float deltaTime)
{
    Vector2 desired = computeLaggedCenter();
    Vector2 current = getPosition();
    const float lerpFactor = std::clamp(mConfig.lerpSpeed * deltaTime, 0.0f, 1.0f);
    Vector2 newPosition = {
        current.x + (desired.x - current.x) * lerpFactor,
        current.y + (desired.y - current.y) * lerpFactor
    };

    setPosition(applyBobOffset(newPosition, deltaTime));
}

Vector2 MusicNote::computeLaggedCenter() const
{
    Entity *parent = getParent();
    if (!parent)
    {
        return getPosition();
    }

    const Vector2 parentPosition = parent->getPosition();
    const Vector2 parentVelocity = parent->getVelocity();
    return {
        parentPosition.x - parentVelocity.x * mConfig.lagCoefficient,
        parentPosition.y - parentVelocity.y * mConfig.lagCoefficient
    };
}

Vector2 MusicNote::computeOrbitTarget() const
{
    const Vector2 center = computeLaggedCenter();
    const float slots = static_cast<float>(std::max<size_t>(1u, mOrbitSlotCount));
    const float slotAngle = (slots > 0.0f)
        ? (static_cast<float>(mOrbitSlotIndex) / slots) * (2.0f * PI)
        : 0.0f;
    const float angle = slotAngle + mOrbitAngle;
    return {
        center.x + cosf(angle) * mOrbitConfig.radius,
        center.y + sinf(angle) * mOrbitConfig.radius
    };
}

Vector2 MusicNote::applyBobOffset(const Vector2 &position, float deltaTime)
{
    Vector2 result = position;
    mBobTimer += deltaTime;
    const float bobOffset = sinf(mBobTimer * mConfig.bobSpeed) * mConfig.bobAmplitude;
    result.y += bobOffset;
    return result;
}

