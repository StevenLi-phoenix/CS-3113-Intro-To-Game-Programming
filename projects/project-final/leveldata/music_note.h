#ifndef MUSIC_NOTE_H
#define MUSIC_NOTE_H

#include "../lib/Entity.h"
#include "../lib/ResourceManager.h"
#include "../constants.h"

#include <cstddef>
#include <vector>

class MusicNote : public Entity
{
public:
    enum class Variant
    {
        Note1 = 0,
        Note2,
        Note3,
        Star
    };

    struct FollowConfig
    {
        float lagCoefficient;
        float lerpSpeed;
        float bobAmplitude;
        float bobSpeed;

        FollowConfig(
            float lag = combat::NOTE_FOLLOW_LAG,
            float lerp = combat::NOTE_FOLLOW_LERP,
            float bobAmp = combat::NOTE_BOB_AMPLITUDE,
            float bobSpd = combat::NOTE_BOB_SPEED)
            : lagCoefficient(lag),
              lerpSpeed(lerp),
              bobAmplitude(bobAmp),
              bobSpeed(bobSpd)
        {}
    };

    struct OrbitConfig
    {
        float radius;
        float angularSpeed;

        OrbitConfig(
            float r = combat::NOTE_ORBIT_RADIUS,
            float speed = combat::NOTE_ORBIT_SPEED)
            : radius(r),
              angularSpeed(speed)
        {}
    };

    struct AttackAnimConfig
    {
        float travelTime;
        float returnTime;

        AttackAnimConfig(
            float travel = combat::NOTE_ATTACK_TRAVEL_TIME,
            float ret = combat::NOTE_ATTACK_RETURN_TIME)
            : travelTime(travel),
              returnTime(ret)
        {}
    };

    MusicNote(Variant variant,
              Entity *parent,
              FollowConfig config = FollowConfig{},
              OrbitConfig orbit = OrbitConfig{});

    void setVariant(Variant variant);
    Variant getVariant() const { return mVariant; }

    int getDamage() const;
    static int damageForVariant(Variant variant);

    void setFollowConfig(const FollowConfig &config) { mConfig = config; }
    void setOrbitConfig(const OrbitConfig &config) { mOrbitConfig = config; }
    void setOrbitSlot(size_t slotIndex, size_t slotCount);
    void setOrbitSuppressed(bool suppressed);
    bool isAvailableForAttack() const;
    void launchAttack(const Vector2 &impactPoint, AttackAnimConfig animConfig = AttackAnimConfig{});

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;

private:
    enum class State
    {
        Orbit,
        AttackOut,
        AttackReturn
    };

    void refreshSprite();
    void updateOrbit(float deltaTime);
    void updateAttackOut(float deltaTime);
    void updateAttackReturn(float deltaTime);
    void updateFollowLag(float deltaTime);
    Vector2 computeLaggedCenter() const;
    Vector2 computeOrbitTarget() const;
    Vector2 applyBobOffset(const Vector2 &position, float deltaTime);
    static const char *tagForVariant(Variant variant);

    Variant mVariant;
    FollowConfig mConfig;
    OrbitConfig mOrbitConfig;
    State mState = State::Orbit;
    size_t mOrbitSlotIndex = 0u;
    size_t mOrbitSlotCount = 1u;
    float mOrbitAngle = 0.0f;
    bool mOrbitSuppressed = false;
    float mBobTimer = 0.0f;
    Vector2 mAttackStart = {0.0f, 0.0f};
    Vector2 mAttackImpact = {0.0f, 0.0f};
    float mAttackTimer = 0.0f;
    float mAttackDuration = 0.0f;
    float mAttackReturnDuration = 0.0f;
};

#endif // MUSIC_NOTE_H

