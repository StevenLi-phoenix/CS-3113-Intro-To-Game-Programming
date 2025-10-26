/**
* Author: Steven Li
* Assignment: Lunar Lander
* Date due: 2025-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
* Author: Steven Li
* Time: 2025/10/22 15:19PM
* Time: 2025/10/25 18:44PM
* Time: 2025/10/26 13:00PM
* Assignment link: https://brightspace.nyu.edu/d2l/lms/dropbox/user/folder_submit_files.d2l?ou=501465&db=1079374
* File UUID: 30c1d136-120c-4a38-96e5-817de24526df
*/

#include "CS3113/Entity.h"
#include "lib/vector_ops.h" // this was intentionally included
#include <algorithm>
#include <cmath>
#include <cstdio>

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#000718";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float ACCELERATION_OF_GRAVITY = 16.35f; // 1/6 of gravity on Earth
constexpr float END_GAME_THRESHOLD      = -800.0f
                  
constexpr float   TRANSLATIONAL_THRUST = 200.0f;
constexpr float   ROTATIONAL_THRUST = 50.0f;
constexpr float   TRANSLATIONAL_STABILISER_THRUST = 10.0f;
constexpr float   ROTATIONAL_STABILISER_THRUST = 10.0f;
constexpr float   TRANSLATIONAL_STABILISER_MINIMUM_SPEED = 0.25f;
constexpr float   ROTATIONAL_STABILISER_MINIMUM_SPEED = 0.25f;
constexpr float   LUNAR_LANDER_MASS = 1.0f;
constexpr float   LUNAR_LANDER_MOMENT = 1.0f;
constexpr float   STABILISER_OVERRIDE_DURATION = 0.25f;
constexpr float   MAIN_THRUSTER_GAP = 6.0f;
constexpr float   MAIN_ENGINE_MIN_THROTTLE = 0.15f;
constexpr float   MAIN_ENGINE_MAX_THROTTLE = 1.0f;
constexpr float   MAIN_ENGINE_DEFAULT_THROTTLE = 0.35f;
constexpr float   MAIN_ENGINE_MAX_THRUST = 400.0f;
constexpr float   MAIN_ENGINE_FUEL_CAPACITY = 200.0f;
constexpr float   MAIN_ENGINE_MAX_FUEL_BURN_RATE = 5.0f;
constexpr float   MAIN_ENGINE_MIN_FUEL_CONSUMPTION_SCALE = 0.30f;
constexpr float   MAIN_ENGINE_FUEL_MASS_PER_UNIT = 0.02f;
constexpr float   MAIN_ENGINE_THROTTLE_STEP = 0.05f;
constexpr float   BACKGROUND_PARALLAX_FACTOR = 0.25f;
constexpr float   BACKGROUND_VELOCITY_OFFSET_SCALE = 0.08f;
constexpr float   CAMERA_ZOOM_MIN = 0.35f;
constexpr float   CAMERA_ZOOM_MAX = 2.0f;
constexpr float   CAMERA_ZOOM_SCROLL_STEP = 0.08f;
constexpr float   CAMERA_MAP_VIEW_ZOOM = 0.45f;
constexpr float   TERRAIN_TILE_WORLD_SIZE = 64.0f;
constexpr float   TERRAIN_BASE_HEIGHT = ORIGIN.y + 140.0f;
constexpr float   TERRAIN_NOISE_AMPLITUDE_PRIMARY = 90.0f;
constexpr float   TERRAIN_NOISE_AMPLITUDE_SECONDARY = 35.0f;
constexpr float   TERRAIN_NOISE_FREQUENCY_PRIMARY = 0.0045f;
constexpr float   TERRAIN_NOISE_FREQUENCY_SECONDARY = 0.0017f;
constexpr float   TERRAIN_MIN_HEIGHT = ORIGIN.y - 80.0f;
constexpr float   TERRAIN_MAX_HEIGHT = SCREEN_HEIGHT + 220.0f;
constexpr int     TERRAIN_ATLAS_COLUMNS = 8;
constexpr int     TERRAIN_ATLAS_ROWS    = 6;
constexpr float   ISS_SPEED = 60.0f;
constexpr float   ISS_VERTICAL_OFFSET = -420.0f;
constexpr float   ISS_BASELINE_Y = ORIGIN.y + ISS_VERTICAL_OFFSET;
constexpr int     ENGINE_MAX_CHANGES = 3;
constexpr float   ENGINE_START_TIME = 1.4f;
constexpr float   ENGINE_CLOSE_TIME = 1.0f;
constexpr float   ENGINE_CHANGE_RATE = 0.5f; // 50% per second
constexpr float   FLAME_ANIMATION_BASE_FPS = 8.0f;
constexpr float   CONTACT_EPSILON = 5.0f;
constexpr float   LANDER_FOOT_SPAN_RATIO = 0.65f;
constexpr float   CAMERA_PAN_LERP_SPEED = 0.1f;
constexpr float   MAX_SAFE_LANDING_SPEED = 25.0f;

static float normaliseAngle(float angle)
{
    angle = std::fmod(angle + 180.0f, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle - 180.0f;
}

static Vector2 rotatePoint(const Vector2 &point, float angleDegrees)
{
    float radians = angleDegrees * DEG2RAD;
    float cosA = cosf(radians);
    float sinA = sinf(radians);
    return {
        point.x * cosA - point.y * sinA,
        point.x * sinA + point.y * cosA
    };
}

class Lander : public Entity
{
public:
    Lander(Vector2 position, Vector2 scale, const char *textureFilepath);
    void toggleTranslationStabiliser();
    void toggleRotationStabiliser();
    bool isTranslationStabiliserEnabled() const { return mTranslationStabiliserEnabled; }
    bool isRotationStabiliserEnabled()   const { return mRotationStabiliserEnabled;   }
    void resetControllers();

    void update(float deltaTime) override;

public:
    void applyTranslationStabilisation(float deltaTime);
    void applyRotationStabilisation(float deltaTime);
    void setTranslationOverrideActive(bool active, float duration = 0.2f);
    void setRotationOverrideActive(bool active, float duration = 0.2f);
    void setTranslationStabiliserEnabled(bool enabled);
    void setRotationStabiliserEnabled(bool enabled);
    void setManualTranslationForce(Vector2 force);
    void setManualRotationTorque(float torque);
    Vector2 getManualTranslationForce() const { return mLastManualTranslationForce; }
    float   getManualRotationTorque()   const { return mLastManualTorque; }
    Vector2 getTotalTranslationForce()  const;
    float   getTotalRotationTorque()    const;
    float   getDesiredAngle()           const { return mDesiredAngle; }
    void toggleMainEngine();
    void adjustMainEngineThrottle(float delta);
    void setMainEngineThrottle(float throttle);
    bool isMainEngineFiring()  const;
    float getMainEngineThrottle() const { return mTargetThrottle; }
    float getMainEngineThrottleNormalised() const;
    float getMainEngineAppliedThrottle() const { return mCurrentThrottle; }
    float getMainEngineAppliedThrottleNormalised() const;
    float getMainEngineOutputLevel() const { return getEngineOutputScalar(); }
    float getFuel() const { return mFuel; }
    float getFuelCapacity() const { return mFuelCapacity; }
    bool hasFuel() const { return mFuel > 0.0f; }
    void setDryMass(float dryMass);
    int getIgnitionsRemaining() const { return mIgnitionsRemaining; }
    int getMaxIgnitions() const { return ENGINE_MAX_CHANGES; }

    float getLastTranslationCorrectionX() const { return mLastTranslationCorrectionX; }
    float getLastTranslationCorrectionY() const { return mLastTranslationCorrectionY; }
    float getLastRotationCorrection()     const { return mLastRotationCorrection;     }
    bool  isTranslationOverrideActive()   const { return mTranslationOverrideActive;  }
    bool  isRotationOverrideActive()      const { return mRotationOverrideActive;     }
    bool  isTranslationStabiliserDesired() const { return mTranslationStabiliserDesired; }
    bool  isRotationStabiliserDesired()    const { return mRotationStabiliserDesired;  }

private:
    enum class MainEngineState { Off, Starting, Running, Stopping };

    bool mTranslationStabiliserEnabled = true;
    bool mRotationStabiliserEnabled    = true;
    bool mTranslationStabiliserDesired = true;
    bool mRotationStabiliserDesired    = true;
    Vector2 mLastManualTranslationForce {0.0f, 0.0f};
    float   mLastManualTorque           = 0.0f;
    float mLastTranslationCorrectionX  = 0.0f;
    float mLastTranslationCorrectionY  = 0.0f;
    float mLastRotationCorrection      = 0.0f;
    bool mTranslationOverrideActive    = false;
    bool mRotationOverrideActive       = false;
    float mTranslationOverrideTimer    = 0.0f;
    float mRotationOverrideTimer       = 0.0f;
    float mDesiredAngle                = 0.0f;
    MainEngineState mEngineState       = MainEngineState::Off;
    float mFuelCapacity                = MAIN_ENGINE_FUEL_CAPACITY;
    float mFuel                        = MAIN_ENGINE_FUEL_CAPACITY;
    float mDryMass                     = LUNAR_LANDER_MASS;
    float mTargetThrottle              = MAIN_ENGINE_DEFAULT_THROTTLE;
    float mCurrentThrottle             = MAIN_ENGINE_MIN_THROTTLE;
    float mEngineStateTimer            = 0.0f;
    float mEngineStateDuration         = 0.0f;
    int   mIgnitionsRemaining          = ENGINE_MAX_CHANGES;

    void applyMainEngineThrust(float deltaTime);
    void updateEffectiveMass();
    void requestEngineStart();
    void requestEngineShutdown();
    void updateEngineState(float deltaTime);
    float getEngineOutputScalar() const;
    bool  canToggleEngine() const;
};

enum GameStatus { GAME_START, GAME_RUNNING, GAME_PAUSED, GAME_WON, GAME_OVER };

struct GameState
{
    Lander *LunarLander = nullptr;

    GameStatus gameStatus;

    Entity *background = nullptr;
    Entity *mainThrusterFlame = nullptr;
    Entity *iss = nullptr;
    Texture2D terrainTexture {};
    Music bgm;
    Sound engineLoopSound;
};

Lander::Lander(Vector2 position, Vector2 scale, const char *textureFilepath)
    : Entity(position, scale, textureFilepath, ENTITY_PLAYER)
{
    resetControllers();
}

void Lander::toggleTranslationStabiliser()
{
    mTranslationStabiliserDesired = !mTranslationStabiliserDesired;
    if (!mTranslationOverrideActive)
    {
        setTranslationStabiliserEnabled(mTranslationStabiliserDesired);
    }
    else if (mTranslationStabiliserDesired)
    {
        // If user re-enabled while override active, honour request once override ends
        mTranslationOverrideTimer = STABILISER_OVERRIDE_DURATION;
    }
}

void Lander::toggleRotationStabiliser()
{
    mRotationStabiliserDesired = !mRotationStabiliserDesired;
    if (!mRotationOverrideActive)
    {
        setRotationStabiliserEnabled(mRotationStabiliserDesired);
    }
    else if (mRotationStabiliserDesired)
    {
        mRotationOverrideTimer = STABILISER_OVERRIDE_DURATION;
    }
}

void Lander::resetControllers()
{
    mTranslationOverrideActive = false;
    mRotationOverrideActive    = false;
    mTranslationOverrideTimer  = 0.0f;
    mRotationOverrideTimer     = 0.0f;
    mTranslationStabiliserDesired = true;
    mRotationStabiliserDesired    = true;
    mLastManualTranslationForce = {0.0f, 0.0f};
    mLastManualTorque           = 0.0f;
    mLastTranslationCorrectionX = 0.0f;
    mLastTranslationCorrectionY = 0.0f;
    mLastRotationCorrection     = 0.0f;
    mDesiredAngle               = normaliseAngle(getAngle());
    mEngineState                = MainEngineState::Off;
    mEngineStateTimer           = 0.0f;
    mEngineStateDuration        = 0.0f;
    mTargetThrottle             = MAIN_ENGINE_DEFAULT_THROTTLE;
    mCurrentThrottle            = MAIN_ENGINE_MIN_THROTTLE;
    mIgnitionsRemaining         = ENGINE_MAX_CHANGES;
    mFuel                       = mFuelCapacity;
    updateEffectiveMass();
    setTranslationStabiliserEnabled(true);
    setRotationStabiliserEnabled(true);
}

void Lander::setTranslationStabiliserEnabled(bool enabled)
{
    bool stateChanged = (mTranslationStabiliserEnabled != enabled);
    if (stateChanged)
    {
        mTranslationStabiliserEnabled = enabled;
        mLastTranslationCorrectionX = 0.0f;
        mLastTranslationCorrectionY = 0.0f;
    }
}

void Lander::setRotationStabiliserEnabled(bool enabled)
{
    bool stateChanged = (mRotationStabiliserEnabled != enabled);
    if (stateChanged)
    {
        mRotationStabiliserEnabled = enabled;

        if (enabled)
        {
            mDesiredAngle = normaliseAngle(getAngle());
        }
    }

    if (!enabled || stateChanged)
    {
        mLastRotationCorrection = 0.0f;
    }
}

void Lander::setTranslationOverrideActive(bool active, float duration)
{
    if (active)
    {
        if (!mTranslationStabiliserDesired && !mTranslationOverrideActive) return;

        mTranslationOverrideActive = true;
        mTranslationOverrideTimer  = duration;

        setTranslationStabiliserEnabled(false);
    }
    else
    {
        mTranslationOverrideActive = false;
        mTranslationOverrideTimer  = 0.0f;

        setTranslationStabiliserEnabled(mTranslationStabiliserDesired);
    }
}

void Lander::setRotationOverrideActive(bool active, float duration)
{
    if (active)
    {
        if (!mRotationStabiliserDesired && !mRotationOverrideActive) return;

        mRotationOverrideActive = true;
        mRotationOverrideTimer  = duration;

        setRotationStabiliserEnabled(false);
    }
    else
    {
        mRotationOverrideActive = false;
        mRotationOverrideTimer  = 0.0f;
        mDesiredAngle = normaliseAngle(getAngle());
        setRotationStabiliserEnabled(mRotationStabiliserDesired);
    }
}

void Lander::setManualTranslationForce(Vector2 force)
{
    mLastManualTranslationForce = force;
}

void Lander::setManualRotationTorque(float torque)
{
    mLastManualTorque = torque;
}

bool Lander::canToggleEngine() const
{
    if (mEngineState == MainEngineState::Off || mEngineState == MainEngineState::Stopping)
    {
        return (mIgnitionsRemaining > 0) && (mFuel > 0.0f);
    }
    return true;
}

void Lander::toggleMainEngine()
{
    if (mEngineState == MainEngineState::Off || mEngineState == MainEngineState::Stopping)
    {
        if (!canToggleEngine()) return;
        requestEngineStart();
        if (mIgnitionsRemaining > 0) mIgnitionsRemaining--;
    }
    else
    {
        requestEngineShutdown();
    }
}

void Lander::setMainEngineThrottle(float throttle)
{
    float clamped = std::max(MAIN_ENGINE_MIN_THROTTLE,
                             std::min(MAIN_ENGINE_MAX_THROTTLE, throttle));
    mTargetThrottle = clamped;
}

void Lander::adjustMainEngineThrottle(float delta)
{
    setMainEngineThrottle(mTargetThrottle + delta);
}

float Lander::getMainEngineThrottleNormalised() const
{
    float range = MAIN_ENGINE_MAX_THROTTLE - MAIN_ENGINE_MIN_THROTTLE;
    if (range <= 0.0f) return 0.0f;
    float normalised = (mTargetThrottle - MAIN_ENGINE_MIN_THROTTLE) / range;
    return std::max(0.0f, std::min(1.0f, normalised));
}

float Lander::getMainEngineAppliedThrottleNormalised() const
{
    float range = MAIN_ENGINE_MAX_THROTTLE - MAIN_ENGINE_MIN_THROTTLE;
    if (range <= 0.0f) return 0.0f;
    float normalised = (mCurrentThrottle - MAIN_ENGINE_MIN_THROTTLE) / range;
    return std::max(0.0f, std::min(1.0f, normalised));
}

void Lander::setDryMass(float dryMass)
{
    mDryMass = std::max(0.1f, dryMass);
    updateEffectiveMass();
}

Vector2 Lander::getTotalTranslationForce() const
{
    Vector2 total = mLastManualTranslationForce;
    if (mTranslationStabiliserEnabled && !mTranslationOverrideActive)
    {
        total.x += mLastTranslationCorrectionX;
        total.y += mLastTranslationCorrectionY;
    }
    return total;
}

float Lander::getTotalRotationTorque() const
{
    float total = mLastManualTorque;
    if (mRotationStabiliserEnabled && !mRotationOverrideActive)
    {
        total += mLastRotationCorrection;
    }
    return total;
}

void Lander::updateEffectiveMass()
{
    float totalMass = mDryMass + (mFuel * MAIN_ENGINE_FUEL_MASS_PER_UNIT);
    setMass(totalMass);
}

void Lander::requestEngineStart()
{
    if (mFuel <= 0.0f)
    {
        mEngineState = MainEngineState::Off;
        return;
    }
    mEngineState = MainEngineState::Starting;
    mEngineStateTimer = 0.0f;
    mEngineStateDuration = std::max(ENGINE_START_TIME, 0.01f);
}

void Lander::requestEngineShutdown()
{
    if (mEngineState == MainEngineState::Off) return;

    mEngineState = MainEngineState::Stopping;
    mEngineStateTimer = 0.0f;
    mEngineStateDuration = std::max(ENGINE_CLOSE_TIME, 0.01f);
}

void Lander::updateEngineState(float deltaTime)
{
    if (deltaTime < 0.0f) deltaTime = 0.0f;

    // Smooth throttle transitions
    float maxDelta = ENGINE_CHANGE_RATE * deltaTime;
    float diff = mTargetThrottle - mCurrentThrottle;
    if (std::fabs(diff) > maxDelta)
    {
        mCurrentThrottle += maxDelta * (diff > 0.0f ? 1.0f : -1.0f);
    }
    else
    {
        mCurrentThrottle = mTargetThrottle;
    }

    if (mEngineState == MainEngineState::Starting || mEngineState == MainEngineState::Stopping)
    {
        mEngineStateTimer += deltaTime;
        if (mEngineStateTimer >= mEngineStateDuration)
        {
            if (mEngineState == MainEngineState::Starting)
            {
                mEngineState = MainEngineState::Running;
            }
            else
            {
                mEngineState = MainEngineState::Off;
            }
            mEngineStateTimer = 0.0f;
            mEngineStateDuration = 0.0f;
        }
    }

    if (mFuel <= 0.0f && mEngineState != MainEngineState::Off)
    {
        requestEngineShutdown();
    }
}

static float logEase(float t)
{
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return std::log1p(t * 9.0f) / std::log(10.0f);
}

float Lander::getEngineOutputScalar() const
{
    switch (mEngineState)
    {
        case MainEngineState::Off:
            return 0.0f;
        case MainEngineState::Running:
            return 1.0f;
        case MainEngineState::Starting:
            if (mEngineStateDuration <= 0.0f) return 1.0f;
            return logEase(mEngineStateTimer / mEngineStateDuration);
        case MainEngineState::Stopping:
            if (mEngineStateDuration <= 0.0f) return 0.0f;
            return 1.0f - logEase(mEngineStateTimer / mEngineStateDuration);
        default:
            return 0.0f;
    }
}

bool Lander::isMainEngineFiring() const
{
    return (mFuel > 0.0f) && (getEngineOutputScalar() > 0.01f);
}

void Lander::applyMainEngineThrust(float deltaTime)
{
    if (deltaTime <= 0.0f) return;
    if (mFuel <= 0.0f)
    {
        mFuel = 0.0f;
        requestEngineShutdown();
        return;
    }

    float engineScalar = getEngineOutputScalar();
    float throttleRatio = (mCurrentThrottle / MAIN_ENGINE_MAX_THROTTLE) * engineScalar;
    if (throttleRatio <= 0.0f) return;

    float angleRad = getAngle() * DEG2RAD;
    Vector2 thrustDirection { std::sin(angleRad), -std::cos(angleRad) };
    Vector2 desiredThrust = Vector2Scale(thrustDirection, MAIN_ENGINE_MAX_THRUST * throttleRatio);

    float throttleNormalised = getMainEngineAppliedThrottleNormalised() * engineScalar;
    float fuelConsumptionScale = MAIN_ENGINE_MIN_FUEL_CONSUMPTION_SCALE +
        throttleNormalised * (1.0f - MAIN_ENGINE_MIN_FUEL_CONSUMPTION_SCALE);
    float requestedFuel = MAIN_ENGINE_MAX_FUEL_BURN_RATE * fuelConsumptionScale * deltaTime;
    float consumedFuel = std::min(requestedFuel, mFuel);
    float thrustScale = (requestedFuel > 0.0f) ? (consumedFuel / requestedFuel) : 1.0f;

    if (thrustScale > 0.0f)
    {
        applyForce(Vector2Scale(desiredThrust, thrustScale));
        setTranslationOverrideActive(true, STABILISER_OVERRIDE_DURATION);
    }

    mFuel -= consumedFuel;
    if (mFuel < 0.0f) mFuel = 0.0f;
    updateEffectiveMass();

    if (consumedFuel < requestedFuel || mFuel <= 0.0f)
    {
        requestEngineShutdown();
    }
}

void Lander::applyTranslationStabilisation(float /* deltaTime */)
{
    constexpr float INPUT_EPSILON = 0.01f;
    bool manualInputActive = (std::fabs(mLastManualTranslationForce.x) > INPUT_EPSILON) ||
                             (std::fabs(mLastManualTranslationForce.y) > INPUT_EPSILON);

    Vector2 velocity = getVelocity();
    float speedSquared = velocity.x * velocity.x + velocity.y * velocity.y;
    float minSpeed = TRANSLATIONAL_STABILISER_MINIMUM_SPEED;

    if (!manualInputActive && speedSquared > (minSpeed * minSpeed))
    {
        float speed = std::sqrt(speedSquared);
        Vector2 normalisedVelocity { velocity.x / speed, velocity.y / speed };
        mLastTranslationCorrectionX = -normalisedVelocity.x * TRANSLATIONAL_STABILISER_THRUST;
        mLastTranslationCorrectionY = -normalisedVelocity.y * TRANSLATIONAL_STABILISER_THRUST;
        applyForce({mLastTranslationCorrectionX, mLastTranslationCorrectionY});
    }
    else
    {
        mLastTranslationCorrectionX = 0.0f;
        mLastTranslationCorrectionY = 0.0f;
    }
}

void Lander::applyRotationStabilisation(float /* deltaTime */)
{
    constexpr float INPUT_EPSILON = 0.01f;
    bool manualInputActive = std::fabs(mLastManualTorque) > INPUT_EPSILON;
    float angularVelocity = getAngularVelocity();

    if (!manualInputActive && std::fabs(angularVelocity) > ROTATIONAL_STABILISER_MINIMUM_SPEED)
    {
        mLastRotationCorrection = (angularVelocity > 0.0f ? -1.0f : 1.0f) * ROTATIONAL_STABILISER_THRUST;
        applyTorque(mLastRotationCorrection);
    }
    else
    {
        mLastRotationCorrection = 0.0f;
    }
}

void Lander::update(float deltaTime)
{
    if (deltaTime <= 0.0f)
    {
        Entity::update(deltaTime);
        return;
    }

    if (mTranslationOverrideActive)
    {
        mTranslationOverrideTimer -= deltaTime;
        if (mTranslationOverrideTimer <= 0.0f)
        {
            setTranslationOverrideActive(false);
        }
    }
    else
    {
        setTranslationStabiliserEnabled(mTranslationStabiliserDesired);
    }

    if (mRotationOverrideActive)
    {
        mRotationOverrideTimer -= deltaTime;
        if (mRotationOverrideTimer <= 0.0f)
        {
            setRotationOverrideActive(false);
        }
        else
        {
            mDesiredAngle = normaliseAngle(getAngle());
        }
    }
    else
    {
        setRotationStabiliserEnabled(mRotationStabiliserDesired);
    }

    updateEngineState(deltaTime);
    applyMainEngineThrust(deltaTime);

    if (mTranslationStabiliserEnabled)
        applyTranslationStabilisation(deltaTime);

    if (mRotationStabiliserEnabled)
        applyRotationStabilisation(deltaTime);

    Entity::update(deltaTime);
}

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f;
Camera2D gCamera {};
bool gMapViewActive = false;
float gStoredZoomBeforeMap = 1.0f;
Vector2 gBackgroundOffset {0.0f, 0.0f};
bool gLandingOutcomeLocked = false;

GameState g;

#ifdef DEBUG
constexpr float DEBUG_DISPLAY_INTERVAL = 0.15f;
float gDebugDisplayTimer = 0.0f;
char gDebugMessage[128] = "Stab Tx: 0.00 Ty: 0.00 R: 0.00";
#endif

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
static void refreshMainThrusterFlameAppearance();
static void drawControlTooltips();
static void panCamera(Camera2D *camera, const Vector2 *targetPosition);
static void handleCameraZoomInput();
static void updateBackgroundParallax();
static void renderBackground();
static void renderTerrain();
static void updateISS(float deltaTime);
static Vector2 getCameraViewExtents();
static float sampleTerrainHeight(float worldX);
static float sampleTerrainHeight(const Vector2 &worldPosition);
struct TerrainContactInfo
{
    bool leftContact = false;
    bool rightContact = false;
    float impactSpeed = 0.0f;
};
static TerrainContactInfo resolveTerrainCollision(Lander *lander);
static void evaluateLandingOutcome(const TerrainContactInfo &contactInfo, Lander *lander);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "AI");
    InitAudioDevice();

    gCamera.target = ORIGIN;
    gCamera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    gCamera.rotation = 0.0f;
    gCamera.zoom = 1.0f;

    g.bgm = LoadMusicStream("assets/game/01_Among_The_Ruins.wav");
    SetMusicVolume(g.bgm, 0.33f);
    PlayMusicStream(g.bgm);

    g.engineLoopSound = LoadSound("assets/game/sound_rocket_mini.wav");
    SetSoundVolume(g.engineLoopSound, 0.35f);
    g.terrainTexture = LoadTexture("assets/game/moon_8x6.png");
    SetTextureFilter(g.terrainTexture, TEXTURE_FILTER_POINT);

    // ----------- BACKGROUND -----------
    g.background = new Entity(
        {ORIGIN.x, ORIGIN.y},
        {SCREEN_WIDTH, SCREEN_HEIGHT},
        "assets/game/background.png",
        ENTITY_BACKGROUND
    );
    g.background->setEntityType(ENTITY_BACKGROUND);

    // ----------- LUNAR LANDER -----------
    g.LunarLander = new Lander(
        {ORIGIN.x, ORIGIN.y},
        {50.0f, 50.0f},
        "assets/game/lunar_lander.png"
    );

    g.LunarLander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    g.LunarLander->setDryMass(LUNAR_LANDER_MASS);
    g.LunarLander->setMomentOfInertia(LUNAR_LANDER_MOMENT);
    g.LunarLander->setAngularVelocity(0.0f);
    g.LunarLander->setAngularAcceleration(0.0f);

    g.LunarLander->resetControllers();

    Vector2 thrusterScale {
        g.LunarLander->getScale().x * 0.45f,
        g.LunarLander->getScale().y * 1.1f
    };
    std::vector<int> flameFrames {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::map<Direction, std::vector<int>> flameAtlas = {
        {UP, flameFrames},
        {DOWN, flameFrames},
        {LEFT, flameFrames},
        {RIGHT, flameFrames}
    };
    g.mainThrusterFlame = new Entity(
        g.LunarLander->getPosition(),
        thrusterScale,
        "assets/game/flames_stacked.png",
        ATLAS,
        {10.0f, 1.0f},
        flameAtlas,
        ENTITY_BACKGROUND
    );
    g.mainThrusterFlame->setFrameSpeed(static_cast<int>(FLAME_ANIMATION_BASE_FPS));
    g.mainThrusterFlame->setParentEntity(g.LunarLander);
    float thrusterOffsetY = (g.LunarLander->getScale().y * 0.5f) +
                            (thrusterScale.y * 0.5f) - MAIN_THRUSTER_GAP;
    g.mainThrusterFlame->setParentLocalOffset({0.0f, thrusterOffsetY});
    g.mainThrusterFlame->setParentRotationInheritance(true);
    g.mainThrusterFlame->deactivate();

    g.iss = new Entity(
        {ORIGIN.x - SCREEN_WIDTH, ORIGIN.y + ISS_VERTICAL_OFFSET},
        {180.0f, 90.0f},
        "assets/game/ISS.png",
        ENTITY_BACKGROUND
    );
    g.iss->setAcceleration({0.0f, 0.0f});
    g.iss->setVelocity({ISS_SPEED, 0.0f});

    g.gameStatus = GAME_START;

    SetTargetFPS(FPS);
}

void processInput()
{
    handleCameraZoomInput();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    switch (g.gameStatus)
    {
        case GAME_START:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_RUNNING;
            if (g.mainThrusterFlame) g.mainThrusterFlame->deactivate();
            break;
        case GAME_RUNNING:
        {
            float manualTorque = 0.0f;
            if (IsKeyDown(KEY_A)) manualTorque -= ROTATIONAL_THRUST;
            if (IsKeyDown(KEY_D)) manualTorque += ROTATIONAL_THRUST;

            if (manualTorque != 0.0f)
            {
                g.LunarLander->applyTorque(manualTorque);
                g.LunarLander->setRotationOverrideActive(true, STABILISER_OVERRIDE_DURATION);
            }
            else if (g.LunarLander->isRotationOverrideActive())
            {
                g.LunarLander->setRotationOverrideActive(false);
            }
            g.LunarLander->setManualRotationTorque(manualTorque);

            if (IsKeyPressed(KEY_SPACE)) g.LunarLander->toggleMainEngine();
            if (IsKeyPressed(KEY_Z))     g.LunarLander->setMainEngineThrottle(MAIN_ENGINE_MAX_THROTTLE);
            if (IsKeyPressed(KEY_X))     g.LunarLander->setMainEngineThrottle(MAIN_ENGINE_MIN_THROTTLE);
            if (IsKeyPressed(KEY_W))     g.LunarLander->adjustMainEngineThrottle(MAIN_ENGINE_THROTTLE_STEP);
            if (IsKeyPressed(KEY_S))     g.LunarLander->adjustMainEngineThrottle(-MAIN_ENGINE_THROTTLE_STEP);

            Vector2 translationalThrust {0.0f, 0.0f};
            bool translationInput = false;
            if (IsKeyDown(KEY_I) || IsKeyDown(KEY_J) || IsKeyDown(KEY_K) || IsKeyDown(KEY_L))
            {
                float angleRad = g.LunarLander->getAngle() * DEG2RAD;
                float cosA = cosf(angleRad);
                float sinA = sinf(angleRad);

                if (IsKeyDown(KEY_I))
                {
                    translationalThrust = translationalThrust + (TRANSLATIONAL_THRUST * Vector2{ sinA, -cosA });
                    translationInput = true;
                }
                if (IsKeyDown(KEY_K))
                {
                    translationalThrust = translationalThrust + (TRANSLATIONAL_THRUST * Vector2{-sinA,  cosA });
                    translationInput = true;
                }
                if (IsKeyDown(KEY_J))
                {
                    translationalThrust = translationalThrust + (TRANSLATIONAL_THRUST * Vector2{-cosA, -sinA });
                    translationInput = true;
                }
                if (IsKeyDown(KEY_L))
                {
                    translationalThrust = translationalThrust + (TRANSLATIONAL_THRUST * Vector2{ cosA,  sinA });
                    translationInput = true;
                }
            }

            if (translationInput)
            {
                g.LunarLander->applyForce(translationalThrust);
                g.LunarLander->setTranslationOverrideActive(true, STABILISER_OVERRIDE_DURATION);
            }

            g.LunarLander->setManualTranslationForce(translationalThrust);

            if (IsKeyPressed(KEY_G)) g.LunarLander->toggleTranslationStabiliser();
            if (IsKeyPressed(KEY_T)) g.LunarLander->toggleRotationStabiliser();

            if (IsKeyPressed(KEY_P)) g.gameStatus = GAME_PAUSED;
            break;
        }
        case GAME_PAUSED:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_RUNNING;
            if (g.mainThrusterFlame) g.mainThrusterFlame->deactivate();
            break;
        case GAME_WON:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_START;
            if (g.mainThrusterFlame) g.mainThrusterFlame->deactivate();
            break;
        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_START;
            if (g.mainThrusterFlame) g.mainThrusterFlame->deactivate();
            break;
    }
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (deltaTime > 0.02f)
        deltaTime = 0.02f;

    UpdateMusicStream(g.bgm);

    switch (g.gameStatus)
    {
        case GAME_START:
            g.LunarLander->setPosition({ORIGIN.x, ORIGIN.y});
            g.LunarLander->setVelocity({0.0f, 0.0f});
            g.LunarLander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
            g.LunarLander->setAngle(0.0f);
            g.LunarLander->setAngularVelocity(0.0f);
            g.LunarLander->setAngularAcceleration(0.0f);
            g.LunarLander->resetControllers();
            if (g.mainThrusterFlame) g.mainThrusterFlame->deactivate();
            gLandingOutcomeLocked = false;
            break;
        case GAME_RUNNING:
            if (g.LunarLander->getPosition().y < END_GAME_THRESHOLD) 
                g.gameStatus = GAME_OVER;
            g.LunarLander->update(deltaTime);
            if (g.gameStatus == GAME_RUNNING)
            {
                TerrainContactInfo contactInfo = resolveTerrainCollision(g.LunarLander);
                evaluateLandingOutcome(contactInfo, g.LunarLander);
            }
#ifdef DEBUG
            if (g.LunarLander)
            {
                gDebugDisplayTimer -= deltaTime;
                if (gDebugDisplayTimer <= 0.0f)
                {
                    Vector2 pos = g.LunarLander->getPosition();
                    Vector2 totalForce = g.LunarLander->getTotalTranslationForce();
                    float totalTorque = g.LunarLander->getTotalRotationTorque();

                    snprintf(
                        gDebugMessage,
                        sizeof(gDebugMessage),
                        "Pos(%.1f,%.1f) Thrust Tx %.1f Ty %.1f R %.1f | Stabiliser Tx %.1f Ty %.1f R %.1f",
                        pos.x,
                        pos.y,
                        totalForce.x,
                        totalForce.y,
                        totalTorque,
                        g.LunarLander->getLastTranslationCorrectionX(),
                        g.LunarLander->getLastTranslationCorrectionY(),
                        g.LunarLander->getLastRotationCorrection()
                    );
                    gDebugDisplayTimer = DEBUG_DISPLAY_INTERVAL;
                }
            }
#endif
            break;
        case GAME_PAUSED:
            break;
        case GAME_WON:
            break;
        case GAME_OVER:
            break;
    }

    if (g.LunarLander)
    {
        const Vector2 target = g.LunarLander->getPosition();
        panCamera(&gCamera, &target);
    }
    updateBackgroundParallax();
    updateISS(deltaTime);

    bool flameShouldBeActive = false;
    bool engineFiring = false;
    float engineOutputLevel = 0.0f;
    float engineThrottleOutput = 0.0f;
    if (g.gameStatus == GAME_RUNNING && g.LunarLander)
    {
        engineFiring = g.LunarLander->isMainEngineFiring();
        engineOutputLevel = g.LunarLander->getMainEngineOutputLevel();
        engineThrottleOutput = g.LunarLander->getMainEngineAppliedThrottleNormalised() * engineOutputLevel;
        flameShouldBeActive = engineFiring;
    }
    else
    {
        engineOutputLevel = 0.0f;
        engineThrottleOutput = 0.0f;
    }

    if (engineFiring)
    {
        float throttleFactor = engineThrottleOutput;
        float volume = 0.25f + 0.65f * throttleFactor;
        SetSoundVolume(g.engineLoopSound, volume);
        if (!IsSoundPlaying(g.engineLoopSound)) PlaySound(g.engineLoopSound);
    }
    else if (IsSoundPlaying(g.engineLoopSound))
    {
        StopSound(g.engineLoopSound);
    }

    if (g.mainThrusterFlame)
    {
        if (flameShouldBeActive) g.mainThrusterFlame->activate();
        else                     g.mainThrusterFlame->deactivate();
    }

    refreshMainThrusterFlameAppearance();
    if (g.mainThrusterFlame) g.mainThrusterFlame->update(deltaTime);
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));
    BeginMode2D(gCamera);
    renderBackground();
    renderTerrain();
    if (g.iss) g.iss->render();

    if (g.mainThrusterFlame) g.mainThrusterFlame->render();
    if (g.LunarLander) g.LunarLander->render();

    EndMode2D();

    bool translationEnabled = g.LunarLander->isTranslationStabiliserEnabled();
    bool rotationEnabled    = g.LunarLander->isRotationStabiliserEnabled();

    Color translationColor;
    const char *translationStatus;
    if (translationEnabled)
    {
        translationColor = GREEN;
        translationStatus = "Translation Stabiliser: ON";
    }
    else if (g.LunarLander->isTranslationOverrideActive() && g.LunarLander->isTranslationStabiliserDesired())
    {
        translationColor = ORANGE;
        translationStatus = "Translation Stabiliser: OFF (override)";
    }
    else
    {
        translationColor = RED;
        translationStatus = "Translation Stabiliser: OFF";
    }

    DrawText(translationStatus, 20, 20, 20, translationColor);

    Color rotationColor;
    const char *rotationStatus;
    if (rotationEnabled)
    {
        rotationColor = GREEN;
        rotationStatus = "Rotation Stabiliser: ON";
    }
    else if (g.LunarLander->isRotationOverrideActive() && g.LunarLander->isRotationStabiliserDesired())
    {
        rotationColor = ORANGE;
        rotationStatus = "Rotation Stabiliser: OFF (override)";
    }
    else
    {
        rotationColor = RED;
        rotationStatus = "Rotation Stabiliser: OFF";
    }

    DrawText(rotationStatus, 20, 45, 20, rotationColor);

    if (g.LunarLander)
    {
        float fuel        = g.LunarLander->getFuel();
        float fuelCap     = g.LunarLander->getFuelCapacity();
        float fuelPercent = (fuelCap > 0.0f) ? (fuel / fuelCap) * 100.0f : 0.0f;
        DrawText(
            TextFormat("Fuel: %05.1f / %.0f (%.0f%%)", fuel, fuelCap, fuelPercent),
            20,
            70,
            18,
            WHITE
        );
        DrawText(
            TextFormat("Throttle: %.0f%%", g.LunarLander->getMainEngineThrottle() * 100.0f),
            20,
            92,
            18,
            WHITE
        );
        DrawText(
            TextFormat(
                "Ignitions: %d / %d",
                g.LunarLander->getIgnitionsRemaining(),
                g.LunarLander->getMaxIgnitions()
            ),
            20,
            114,
            18,
            WHITE
        );
    }

#ifdef DEBUG
    if (g.LunarLander)
    {
        DrawText(gDebugMessage, 20, 140, 18, YELLOW);
    }
#endif

    switch (g.gameStatus)
    {
        case GAME_START:
            DrawText("Press Enter to Start", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, WHITE);
            break;
        case GAME_PAUSED:
            DrawText("Paused", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, WHITE);
            DrawText("Press Enter to Resume", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20, 20, WHITE);
            break;
        case GAME_WON:
            DrawText("You Won!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, WHITE);
            DrawText("Press Enter to Restart", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20, 20, WHITE);
            break;
        case GAME_OVER:
            DrawText("Game Over", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, WHITE);
            DrawText("Press Enter to Restart", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20, 20, WHITE);
            break;
        case GAME_RUNNING:
        default:
            break;
    }

    drawControlTooltips();

    EndDrawing();
}

static void refreshMainThrusterFlameAppearance()
{
    if (!g.mainThrusterFlame || !g.LunarLander) return;

    float throttleNormalised = g.LunarLander->getMainEngineAppliedThrottleNormalised() *
                               g.LunarLander->getMainEngineOutputLevel();
    Vector2 newScale {
        g.LunarLander->getScale().x * (0.4f + 0.1f * throttleNormalised),
        g.LunarLander->getScale().y * (0.9f + 0.4f * throttleNormalised)
    };
    g.mainThrusterFlame->setScale(newScale);

    float thrusterOffsetY = (g.LunarLander->getScale().y * 0.5f) +
                            (newScale.y * 0.5f) - MAIN_THRUSTER_GAP;
    g.mainThrusterFlame->setParentLocalOffset({0.0f, thrusterOffsetY});
    g.mainThrusterFlame->setParentRotationInheritance(true);

    int frameSpeed = static_cast<int>(FLAME_ANIMATION_BASE_FPS + throttleNormalised * FLAME_ANIMATION_BASE_FPS);
    if (frameSpeed < 4) frameSpeed = 4;
    g.mainThrusterFlame->setFrameSpeed(frameSpeed);
}

static void drawControlTooltips()
{
    const int baseY = SCREEN_HEIGHT - 75;
    int ignitionsRemaining = g.LunarLander ? g.LunarLander->getIgnitionsRemaining() : ENGINE_MAX_CHANGES;
    int ignitionsMax = g.LunarLander ? g.LunarLander->getMaxIgnitions() : ENGINE_MAX_CHANGES;
    switch (g.gameStatus)
    {
        case GAME_START:
            DrawText("Enter: Start mission   |   Q: Quit", 20, baseY, 18, LIGHTGRAY);
            DrawText(
                TextFormat(
                    "Space: Ignite main engine (%d/%d ignitions)   |   W/S: Adjust throttle   |   Z/X: Max/Min",
                    ignitionsRemaining,
                    ignitionsMax
                ),
                20,
                baseY + 22,
                16,
                LIGHTGRAY
            );
            DrawText("A/D: Rotate   |   IJKL: RCS translation   |   G/T: Stabiliser toggles   |   Scroll: Zoom   |   M: Map view", 20, baseY + 42, 16, LIGHTGRAY);
            break;
        case GAME_RUNNING:
            DrawText(
                TextFormat(
                    "Space: Ignite engine (%d/%d left)   |   W/S: Throttle +/-   |   Z: Max   |   X: Min",
                    ignitionsRemaining,
                    ignitionsMax
                ),
                20,
                baseY,
                16,
                LIGHTGRAY
            );
            DrawText("IJKL: RCS translation   |   A/D: Rotate   |   P: Pause   |   Q: Quit", 20, baseY + 20, 16, LIGHTGRAY);
            DrawText("G/T: Toggle stabilisers   |   Scroll: Zoom   |   M: Map view", 20, baseY + 40, 16, LIGHTGRAY);
            break;
        case GAME_PAUSED:
            DrawText("Paused: Enter resumes   |   Q quits", 20, baseY, 18, LIGHTGRAY);
            break;
        case GAME_WON:
        case GAME_OVER:
            DrawText("Enter: Restart   |   Q: Quit", 20, baseY, 18, LIGHTGRAY);
            break;
    }
}

static void panCamera(Camera2D *camera, const Vector2 *targetPosition)
{
    Vector2 positionDifference = Vector2Subtract(
        *targetPosition,
        camera->target
    );

    camera->target = Vector2Add(
        camera->target,
        Vector2Scale(positionDifference, CAMERA_PAN_LERP_SPEED)
    );
}

static void updateBackgroundParallax()
{
    if (!g.background || !g.LunarLander) return;

    Vector2 cameraRelativeOffset = Vector2Subtract(gCamera.target, ORIGIN);
    Vector2 parallaxOffset = Vector2Scale(cameraRelativeOffset, BACKGROUND_PARALLAX_FACTOR);
    Vector2 velocityOffset = Vector2Scale(g.LunarLander->getVelocity(), BACKGROUND_VELOCITY_OFFSET_SCALE);
    gBackgroundOffset = Vector2Add(parallaxOffset, velocityOffset);
}

static float clampCameraZoom(float value)
{
    if (value < CAMERA_ZOOM_MIN) return CAMERA_ZOOM_MIN;
    if (value > CAMERA_ZOOM_MAX) return CAMERA_ZOOM_MAX;
    return value;
}

static void handleCameraZoomInput()
{
    float wheelDelta = GetMouseWheelMove();
    if (wheelDelta != 0.0f)
    {
        if (gMapViewActive)
        {
            gStoredZoomBeforeMap = clampCameraZoom(
                gStoredZoomBeforeMap + wheelDelta * CAMERA_ZOOM_SCROLL_STEP
            );
        }
        else
        {
            gCamera.zoom = clampCameraZoom(
                gCamera.zoom + wheelDelta * CAMERA_ZOOM_SCROLL_STEP
            );
            gStoredZoomBeforeMap = gCamera.zoom;
        }
    }

    if (IsKeyPressed(KEY_M))
    {
        gMapViewActive = !gMapViewActive;
        if (gMapViewActive)
        {
            gStoredZoomBeforeMap = clampCameraZoom(gCamera.zoom);
            gCamera.zoom = clampCameraZoom(CAMERA_MAP_VIEW_ZOOM);
        }
        else
        {
            gCamera.zoom = clampCameraZoom(gStoredZoomBeforeMap);
        }
    }

    if (gMapViewActive)
    {
        gCamera.zoom = clampCameraZoom(CAMERA_MAP_VIEW_ZOOM);
    }
    else
    {
        gCamera.zoom = clampCameraZoom(gCamera.zoom);
    }
}

static Vector2 getCameraViewExtents()
{
    float zoom = (gCamera.zoom <= 0.0f) ? 0.0001f : gCamera.zoom;
    return {
        (SCREEN_WIDTH / zoom) / 2.0f,
        (SCREEN_HEIGHT / zoom) / 2.0f
    };
}

static unsigned int terrainHash(int column, int depth)
{
    unsigned int h = static_cast<unsigned int>(column) * 374761393u;
    h += static_cast<unsigned int>(depth) * 668265263u;
    h ^= h >> 13;
    h *= 1274126177u;
    return h;
}

static float sampleTerrainHeight(float worldX)
{
    float primary = std::sinf(worldX * TERRAIN_NOISE_FREQUENCY_PRIMARY) * TERRAIN_NOISE_AMPLITUDE_PRIMARY;
    float secondary = std::sinf(worldX * TERRAIN_NOISE_FREQUENCY_SECONDARY + 1.3f) * TERRAIN_NOISE_AMPLITUDE_SECONDARY;
    float height = TERRAIN_BASE_HEIGHT + primary + secondary;
    if (height < TERRAIN_MIN_HEIGHT) height = TERRAIN_MIN_HEIGHT;
    if (height > TERRAIN_MAX_HEIGHT) height = TERRAIN_MAX_HEIGHT;
    float tileHeight = TERRAIN_TILE_WORLD_SIZE;
    float snappedTopOfTile = std::floor(height / tileHeight) * tileHeight;
    return snappedTopOfTile;
}

static float sampleTerrainHeight(const Vector2 &worldPosition)
{
    return sampleTerrainHeight(worldPosition.x);
}

static Rectangle getTerrainSourceRect(int row, int col)
{
    if (row < 0) row = 0;
    if (row >= TERRAIN_ATLAS_ROWS) row = TERRAIN_ATLAS_ROWS - 1;
    int wrappedCol = ((col % TERRAIN_ATLAS_COLUMNS) + TERRAIN_ATLAS_COLUMNS) % TERRAIN_ATLAS_COLUMNS;
    int index = row * TERRAIN_ATLAS_COLUMNS + wrappedCol;
    return getUVRectangle(&g.terrainTexture, index, TERRAIN_ATLAS_ROWS, TERRAIN_ATLAS_COLUMNS);
}

static TerrainContactInfo resolveTerrainCollision(Lander *lander)
{
    TerrainContactInfo info;
    if (!lander) return info;

    const Vector2 scale = lander->getScale();
    Vector2 position = lander->getPosition();
    const float angle = lander->getAngle();
    const float halfWidth  = scale.x * 0.5f;
    const float halfHeight = scale.y * 0.5f;

    const Vector2 leftLocal  { -halfWidth * LANDER_FOOT_SPAN_RATIO,  halfHeight };
    const Vector2 rightLocal {  halfWidth * LANDER_FOOT_SPAN_RATIO,  halfHeight };

    Vector2 leftFootWorld  = position + rotatePoint(leftLocal, angle);
    Vector2 rightFootWorld = position + rotatePoint(rightLocal, angle);

    const float leftSurfaceHeight  = sampleTerrainHeight(leftFootWorld);
    const float rightSurfaceHeight = sampleTerrainHeight(rightFootWorld);
    const float leftPenetration    = leftFootWorld.y  - leftSurfaceHeight;
    const float rightPenetration   = rightFootWorld.y - rightSurfaceHeight;
    const float penetration        = std::max(leftPenetration, rightPenetration);

    const Vector2 velocityBefore = lander->getVelocity();
    const float impactSpeed = std::sqrt(velocityBefore.x * velocityBefore.x +
                                        velocityBefore.y * velocityBefore.y);

    if (penetration > 0.0f)
    {
        position.y -= penetration;
        leftFootWorld.y  -= penetration;
        rightFootWorld.y -= penetration;
        lander->setPosition(position);

        Vector2 velocity = lander->getVelocity();
        if (velocity.y > 0.0f) velocity.y = 0.0f;
        lander->setVelocity(velocity);
    }

    const float leftOffsetAfter  = leftFootWorld.y  - leftSurfaceHeight;
    const float rightOffsetAfter = rightFootWorld.y - rightSurfaceHeight;
    info.leftContact  = std::fabs(leftOffsetAfter)  <= CONTACT_EPSILON;
    info.rightContact = std::fabs(rightOffsetAfter) <= CONTACT_EPSILON;
    if (info.leftContact || info.rightContact)
        info.impactSpeed = impactSpeed;

    return info;
}

static void evaluateLandingOutcome(const TerrainContactInfo &contactInfo, Lander *lander)
{
    if (!lander || gLandingOutcomeLocked || g.gameStatus != GAME_RUNNING) return;

    bool anyContact = contactInfo.leftContact || contactInfo.rightContact;
    if (!anyContact) return;

    if (lander->isMainEngineFiring()) lander->toggleMainEngine();
    lander->setMainEngineThrottle(MAIN_ENGINE_MIN_THROTTLE);
    gLandingOutcomeLocked = true;

    float speed = contactInfo.impactSpeed;
    if (speed <= 0.0f)
    {
        Vector2 velocity = lander->getVelocity();
        speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    }
    if (contactInfo.leftContact && contactInfo.rightContact && speed < MAX_SAFE_LANDING_SPEED)
    {
        g.gameStatus = GAME_WON;
    }
    else
    {
        g.gameStatus = GAME_OVER;
    }
}

static void renderTerrain()
{
    if (g.terrainTexture.id == 0) return;

    Vector2 extents = getCameraViewExtents();
    float viewMinX = gCamera.target.x - extents.x;
    float viewMaxX = gCamera.target.x + extents.x;
    float viewMaxY = gCamera.target.y + extents.y;

    float tileWidth = TERRAIN_TILE_WORLD_SIZE;
    float tileHeight = TERRAIN_TILE_WORLD_SIZE;

    int startColumn = static_cast<int>(std::floor(viewMinX / tileWidth)) - 2;
    int endColumn   = static_cast<int>(std::ceil(viewMaxX / tileWidth)) + 2;

    Vector2 origin { tileWidth / 2.0f, tileHeight / 2.0f };

    for (int column = startColumn; column <= endColumn; ++column)
    {
        float columnWorldStart = column * tileWidth;
        float columnCenterX = columnWorldStart + tileWidth / 2.0f;
        float surfaceHeight = sampleTerrainHeight(columnCenterX);
        float snappedTop = std::floor(surfaceHeight / tileHeight) * tileHeight + tileHeight / 2.0f;

        unsigned int surfaceHash = terrainHash(column, 0);
        Rectangle surfaceSource = getTerrainSourceRect(0, surfaceHash % TERRAIN_ATLAS_COLUMNS);
        Rectangle surfaceDest {
            columnCenterX,
            snappedTop,
            tileWidth,
            tileHeight
        };
        DrawTexturePro(g.terrainTexture, surfaceSource, surfaceDest, origin, 0.0f, WHITE);

        int depth = 1;
        for (float y = snappedTop + tileHeight; y <= viewMaxY + tileHeight; y += tileHeight, ++depth)
        {
            unsigned int groundHash = terrainHash(column, depth);
            int groundRow = 1 + (groundHash % (TERRAIN_ATLAS_ROWS - 1));
            int groundCol = (groundHash / 7) % TERRAIN_ATLAS_COLUMNS;
            Rectangle groundSource = getTerrainSourceRect(groundRow, groundCol);
            Rectangle groundDest {
                columnCenterX,
                y,
                tileWidth,
                tileHeight
            };
            DrawTexturePro(g.terrainTexture, groundSource, groundDest, origin, 0.0f, WHITE);
        }
    }
}

static void updateISS(float deltaTime)
{
    if (!g.iss) return;

    Vector2 position = g.iss->getPosition();
    position.y = ISS_BASELINE_Y;
    g.iss->setPosition(position);

    if (deltaTime <= 0.0f) return;

    g.iss->setAcceleration({0.0f, 0.0f});
    g.iss->setVelocity({ISS_SPEED, 0.0f});
    g.iss->update(deltaTime);

    Vector2 extents = getCameraViewExtents();
    float worldRight = gCamera.target.x + extents.x;
    float worldLeft  = gCamera.target.x - extents.x;
    float halfWidth  = g.iss->getScale().x / 2.0f;
    float margin     = halfWidth * 2.0f;

    if (g.iss->getPosition().x - halfWidth > worldRight + margin)
    {
        g.iss->setPosition({worldLeft - margin, ISS_BASELINE_Y});
    }

    if (g.gameStatus == GAME_RUNNING && g.LunarLander)
    {
        Rectangle issRect {
            g.iss->getPosition().x - halfWidth,
            g.iss->getPosition().y - g.iss->getScale().y / 2.0f,
            g.iss->getScale().x,
            g.iss->getScale().y
        };

        Vector2 landerScale = g.LunarLander->getScale();
        Rectangle landerRect {
            g.LunarLander->getPosition().x - landerScale.x / 2.0f,
            g.LunarLander->getPosition().y - landerScale.y / 2.0f,
            landerScale.x,
            landerScale.y
        };

        if (CheckCollisionRecs(issRect, landerRect))
        {
            g.gameStatus = GAME_OVER;
            gLandingOutcomeLocked = true;
            if (g.LunarLander->isMainEngineFiring()) g.LunarLander->toggleMainEngine();
        }
    }
}

static void renderBackground()
{
    if (!g.background) return;

    Texture2D texture = g.background->getTexture();
    if (texture.id == 0) return;

    Vector2 tileSize {
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    if (tileSize.x <= 0.0f || tileSize.y <= 0.0f) return;

    Vector2 extents = getCameraViewExtents();
    Vector2 viewMin {
        gCamera.target.x - extents.x,
        gCamera.target.y - extents.y
    };
    Vector2 viewMax {
        gCamera.target.x + extents.x,
        gCamera.target.y + extents.y
    };

    Vector2 offset = gBackgroundOffset;

    float startX = std::floor((viewMin.x + offset.x) / tileSize.x) * tileSize.x;
    float startY = std::floor((viewMin.y + offset.y) / tileSize.y) * tileSize.y;

    Rectangle source {
        0.0f,
        0.0f,
        tileSize.x,
        tileSize.y
    };

    for (float x = startX; x < viewMax.x + offset.x + tileSize.x; x += tileSize.x)
    {
        for (float y = startY; y < viewMax.y + offset.y + tileSize.y; y += tileSize.y)
        {
            float drawCenterX = (x - offset.x) + tileSize.x / 2.0f;
            float drawCenterY = (y - offset.y) + tileSize.y / 2.0f;

            Rectangle dest {
                drawCenterX,
                drawCenterY,
                tileSize.x,
                tileSize.y
            };

            Vector2 origin { tileSize.x / 2.0f, tileSize.y / 2.0f };
            DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
        }
    }
}

void shutdown()
{
    delete g.mainThrusterFlame;
    delete g.LunarLander;
    delete g.background;
    delete g.iss;

    if (g.terrainTexture.id != 0)
    {
        UnloadTexture(g.terrainTexture);
        g.terrainTexture.id = 0;
    }

    StopMusicStream(g.bgm);
    StopSound(g.engineLoopSound);
    UnloadMusicStream(g.bgm);
    UnloadSound(g.engineLoopSound);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
