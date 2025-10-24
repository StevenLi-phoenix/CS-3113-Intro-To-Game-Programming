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
* Assignment link: https://brightspace.nyu.edu/d2l/lms/dropbox/user/folder_submit_files.d2l?ou=501465&db=1079374
* File UUID: 30c1d136-120c-4a38-96e5-817de24526df
*/

#include "CS3113/Entity.h"
#include "lib/pid_controller.h"
#include "lib/vector_ops.h"
#include <cmath>
#include <cstdio>
#include <iostream>

static float normaliseAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
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

    void setTranslationPIDGains(float kp, float ki, float kd);
    void setRotationPIDGains(float kp, float ki, float kd);
    void setTranslationIntegralLimit(float limit);
    void setRotationIntegralLimit(float limit);
    void setTranslationOutputLimit(float limit);
    void setRotationOutputLimit(float limit);

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

    float getLastTranslationCorrectionX() const { return mLastTranslationCorrectionX; }
    float getLastTranslationCorrectionY() const { return mLastTranslationCorrectionY; }
    float getLastRotationCorrection()     const { return mLastRotationCorrection;     }
    bool  isTranslationOverrideActive()   const { return mTranslationOverrideActive;  }
    bool  isRotationOverrideActive()      const { return mRotationOverrideActive;     }
    bool  isTranslationStabiliserDesired() const { return mTranslationStabiliserDesired; }
    bool  isRotationStabiliserDesired()    const { return mRotationStabiliserDesired;  }

private:
    PIDController mVelocityPID_X;
    PIDController mVelocityPID_Y;
    PIDController mAngularPID;

    float mTranslationOutputLimit  = 900.0f;
    float mRotationOutputLimit     = 400.0f;

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
};

enum GameStatus { GAME_START, GAME_RUNNING, GAME_PAUSED, GAME_WON, GAME_OVER };

struct GameState
{
    // Entity *xochitl;
    // Entity *tiles;
    // Entity *blocks;
    // Entity *ghost;

    Lander *LunarLander;

    GameStatus gameStatus;

    Music bgm;
    Sound burstSound;
};

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#000718";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 6, 8 };
constexpr float   TRANSLATIONAL_THRUST = 200.0f;
constexpr float   ROTATIONAL_THRUST = 50.0f;
constexpr float   LUNAR_LANDER_MASS = 1.0f;
constexpr float   LUNAR_LANDER_MOMENT = 1.0f;
constexpr float   VELOCITY_PID_KP        = 1.0f;
constexpr float   VELOCITY_PID_KI        = 1.0f;
constexpr float   VELOCITY_PID_KD        = 10.0f;
constexpr float   VELOCITY_PID_INTEGRAL_LIMIT = 800.0f;
constexpr float   VELOCITY_PID_MAX_OUTPUT     = 400.0f;
constexpr float   ROTATION_PID_KP             = 1.0f;
constexpr float   ROTATION_PID_KI             = 1.0f;
constexpr float   ROTATION_PID_KD             = 10.0f;
constexpr float   ROTATION_PID_INTEGRAL_LIMIT = 400.0f;
constexpr float   ROTATION_PID_MAX_OUTPUT     = 200.0f;
constexpr float   STABILISER_OVERRIDE_DURATION = 0.25f;

// constexpr int   NUMBER_OF_TILES         = 20,
//                 NUMBER_OF_BLOCKS        = 3;
// constexpr float TILE_DIMENSION          = 50.0f,
//                 // in m/ms², since delta time is in ms
//                 ACCELERATION_OF_GRAVITY = 981.0f,
//                 END_GAME_THRESHOLD      = 800.0f;

constexpr float ACCELERATION_OF_GRAVITY = 163.5f; // 1/6 of gravity on Earth
constexpr float END_GAME_THRESHOLD      = 800.0f;

Lander::Lander(Vector2 position, Vector2 scale, const char *textureFilepath)
    : Entity(position, scale, textureFilepath, ENTITY_PLAYER)
{
    setTranslationPIDGains(VELOCITY_PID_KP, VELOCITY_PID_KI, VELOCITY_PID_KD);
    setRotationPIDGains(ROTATION_PID_KP, ROTATION_PID_KI, ROTATION_PID_KD);

    setTranslationIntegralLimit(VELOCITY_PID_INTEGRAL_LIMIT);
    setRotationIntegralLimit(ROTATION_PID_INTEGRAL_LIMIT);

    setTranslationOutputLimit(VELOCITY_PID_MAX_OUTPUT);
    setRotationOutputLimit(ROTATION_PID_MAX_OUTPUT);
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
    mVelocityPID_X.reset();
    mVelocityPID_Y.reset();
    mAngularPID.reset();
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
    setTranslationStabiliserEnabled(true);
    setRotationStabiliserEnabled(true);
}

void Lander::setTranslationStabiliserEnabled(bool enabled)
{
    if (mTranslationStabiliserEnabled != enabled)
    {
        mTranslationStabiliserEnabled = enabled;
        mVelocityPID_X.reset();
        mVelocityPID_Y.reset();
    }

    if (enabled)
    {
        mLastTranslationCorrectionX = 0.0f;
        mLastTranslationCorrectionY = 0.0f;
    }
    else
    {
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
        mAngularPID.reset();

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

void Lander::setTranslationPIDGains(float kp, float ki, float kd)
{
    mVelocityPID_X.setGains(kp, ki, kd);
    mVelocityPID_Y.setGains(kp, ki, kd);
}

void Lander::setRotationPIDGains(float kp, float ki, float kd)
{
    mAngularPID.setGains(kp, ki, kd);
}

void Lander::setTranslationIntegralLimit(float limit)
{
    float absLimit = std::fabs(limit);
    mVelocityPID_X.setIntegralLimits(-absLimit, absLimit);
    mVelocityPID_Y.setIntegralLimits(-absLimit, absLimit);
}

void Lander::setRotationIntegralLimit(float limit)
{
    float absLimit = std::fabs(limit);
    mAngularPID.setIntegralLimits(-absLimit, absLimit);
}

void Lander::setTranslationOutputLimit(float limit)
{
    float absLimit = std::fabs(limit);
    mTranslationOutputLimit = absLimit;
    mVelocityPID_X.setOutputLimits(-absLimit, absLimit);
    mVelocityPID_Y.setOutputLimits(-absLimit, absLimit);
}

void Lander::setRotationOutputLimit(float limit)
{
    float absLimit = std::fabs(limit);
    mRotationOutputLimit = absLimit;
    mAngularPID.setOutputLimits(-absLimit, absLimit);
}

void Lander::applyTranslationStabilisation(float deltaTime)
{
    Vector2 velocity = getVelocity();
    mLastTranslationCorrectionX = mVelocityPID_X.compute(0.0f, velocity.x, deltaTime);
    mLastTranslationCorrectionY = mVelocityPID_Y.compute(0.0f, velocity.y, deltaTime);
    applyForce({mLastTranslationCorrectionX, mLastTranslationCorrectionY});
}

void Lander::applyRotationStabilisation(float deltaTime)
{
    float angle = normaliseAngle(getAngle());
    float error = normaliseAngle(mDesiredAngle - angle);
    mLastRotationCorrection = mAngularPID.compute(error, 0.0f, deltaTime);
    applyTorque(mLastRotationCorrection);
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

    if (mTranslationStabiliserEnabled)
        applyTranslationStabilisation(deltaTime);

    if (mRotationStabiliserEnabled)
        applyRotationStabilisation(deltaTime);

    Entity::update(deltaTime);
}

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f;

GameState g;

#ifdef DEBUG
constexpr float DEBUG_DISPLAY_INTERVAL = 0.15f;
float gDebugDisplayTimer = 0.0f;
char gDebugMessage[128] = "PID Tx: 0.00 Ty: 0.00 R: 0.00";
#endif

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "AI");
    InitAudioDevice();

    g.bgm = LoadMusicStream("assets/game/01_Among_The_Ruins.wav");
    SetMusicVolume(g.bgm, 0.33f);
    PlayMusicStream(g.bgm);

    g.burstSound = LoadSound("assets/game/sound_rocket_mini.wav");

    // /*
    //     ----------- PROTAGONIST -----------
    // */
    // std::map<Direction, std::vector<int>> xochitlAnimationAtlas = {
    //     {DOWN,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
    //     {LEFT,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
    //     {UP,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
    //     {RIGHT, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    // };

    // float sizeRatio  = 48.0f / 64.0f;

    // // Assets from @see https://sscary.itch.io/the-adventurer-female
    // g.xochitl = new Entity(
    //     {ORIGIN.x - 300.0f, ORIGIN.y - 200.0f}, // position
    //     {250.0f * sizeRatio, 250.0f},           // scale
    //     "assets/game/walk.png",                 // texture file address
    //     ATLAS,                                  // single image or atlas?
    //     ATLAS_DIMENSIONS,                       // atlas dimensions
    //     xochitlAnimationAtlas,                  // actual atlas
    //     PLAYER                                  // entity type
    // );

    // g.xochitl->setJumpingPower(450.0f);
    // g.xochitl->setColliderDimensions({
    //     g.xochitl->getScale().x / 3.0f,
    //     g.xochitl->getScale().y / 3.0f
    // });
    // g.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- TILES -----------
    */
    // g.tiles = new Entity[NUMBER_OF_TILES];

    // // Compute the left‑most x coordinate so that the entire row is centred
    // float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;

    // for (int i = 0; i < NUMBER_OF_TILES; i++) 
    // {
    //     // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
    //     g.tiles[i].setTexture("assets/game/tile_0000.png");
    //     g.tiles[i].setEntityType(PLATFORM);
    //     g.tiles[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
    //     g.tiles[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    //     g.tiles[i].setPosition({
    //         leftMostX + i * TILE_DIMENSION, 
    //         ORIGIN.y + TILE_DIMENSION
    //     });
    // }

    /*
        ----------- BLOCKS -----------
    */
    // g.blocks = new Entity[NUMBER_OF_BLOCKS];

    // for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
    // {
    //     // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
    //     g.blocks[i].setTexture("assets/game/tile_0061.png");
    //     g.blocks[i].setEntityType(BLOCK);
    //     g.blocks[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
    //     g.blocks[i].setColliderDimensions(
    //         {TILE_DIMENSION, TILE_DIMENSION});
    // }

    // g.blocks[0].setPosition(
    //     {ORIGIN.x - TILE_DIMENSION * 3, ORIGIN.y - TILE_DIMENSION * 2.5f});
    // g.blocks[1].setPosition(
    //     {ORIGIN.x, ORIGIN.y - TILE_DIMENSION * 2.5f});
    // g.blocks[2].setPosition(
    //     {ORIGIN.x + TILE_DIMENSION * 3, ORIGIN.y - TILE_DIMENSION * 2.5f});


    /*
        ----------- GHOST -----------
    */
    // std::map<Direction, std::vector<int>> ghostAnimationAtlas = {
    //     {LEFT,  { 1, 9, 17, 25 }},
    //     {RIGHT, { 0, 8, 16, 24 }},
    // };

    // // @see dyru.itch.io/pixel-ghost-template
    // g.ghost = new Entity(
    //     {ORIGIN.x + 300.0f, ORIGIN.y - 200.0f}, // position
    //     {100.0f, 100.0f},                       // scale
    //     "assets/game/gosth.png",                // texture file address
    //     ATLAS,                                  // single image or atlas?
    //     ATLAS_DIMENSIONS,                       // atlas dimensions
    //     ghostAnimationAtlas,                    // actual atlas
    //     NPC                                     // entity type
    // );

    // g.ghost->setAIType(FOLLOWER);
    // g.ghost->setAIState(IDLE);
    // g.ghost->setSpeed(Entity::DEFAULT_SPEED * 0.50f);

    // g.ghost->setColliderDimensions({
    //     g.ghost->getScale().x / 2.0f,
    //     g.ghost->getScale().y
    // });

    // g.ghost->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    // g.ghost->setDirection(LEFT);

    // ----------- LUNAR LANDER -----------
    g.LunarLander = new Lander(
        {ORIGIN.x, ORIGIN.y},
        {50.0f, 50.0f},
        "assets/game/lunar_lander.png"
    );

    g.LunarLander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    g.LunarLander->setMass(LUNAR_LANDER_MASS);
    g.LunarLander->setMomentOfInertia(LUNAR_LANDER_MOMENT);
    g.LunarLander->setAngularVelocity(0.0f);
    g.LunarLander->setAngularAcceleration(0.0f);

    g.LunarLander->setTranslationPIDGains(VELOCITY_PID_KP, VELOCITY_PID_KI, VELOCITY_PID_KD);
    g.LunarLander->setRotationPIDGains(ROTATION_PID_KP, ROTATION_PID_KI, ROTATION_PID_KD);
    g.LunarLander->setTranslationIntegralLimit(VELOCITY_PID_INTEGRAL_LIMIT);
    g.LunarLander->setRotationIntegralLimit(ROTATION_PID_INTEGRAL_LIMIT);
    g.LunarLander->setTranslationOutputLimit(VELOCITY_PID_MAX_OUTPUT);
    g.LunarLander->setRotationOutputLimit(ROTATION_PID_MAX_OUTPUT);
    g.LunarLander->resetControllers();

    g.gameStatus = GAME_START;

    // g.ghost->render(); // calling render once at the beginning to switch ghost's direction
    SetTargetFPS(FPS);
}

void processInput()
{
    // g.xochitl->resetMovement();
    // g.ghost->resetMovement();

    // if      (IsKeyDown(KEY_A)) g.xochitl->moveLeft();
    // else if (IsKeyDown(KEY_D)) g.xochitl->moveRight();

    // if (IsKeyPressed(KEY_W) && g.xochitl->isCollidingBottom())
    // {
    //     g.xochitl->jump();
    //     PlaySound(g.burstSound);
    // }

    // Sample keypress for sound effect
    // if (IsKeyPressed(KEY_SPACE)) PlaySound(g.burstSound);

    // // to avoid faster diagonal speed
    // if (GetLength(g.xochitl->getMovement()) > 1.0f) 
    //     g.xochitl->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    switch (g.gameStatus)
    {
        case GAME_START:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_RUNNING;
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
            break;
        case GAME_WON:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_START;
            break;
        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER)) g.gameStatus = GAME_START;
            break;
    }
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

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
            break;
        case GAME_RUNNING:
            if (g.LunarLander->getPosition().y > END_GAME_THRESHOLD) 
                g.gameStatus = GAME_OVER;
            g.LunarLander->update(deltaTime);
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
                        "Pos(%.1f,%.1f) Thrust Tx %.1f Ty %.1f R %.1f | PID Tx %.1f Ty %.1f R %.1f",
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


    // g.xochitl->update(deltaTime, nullptr, g.tiles, 
    //     NUMBER_OF_TILES, g.blocks, NUMBER_OF_BLOCKS);

    // g.ghost->update(deltaTime, g.xochitl, g.tiles, 
    //     NUMBER_OF_TILES, g.blocks, NUMBER_OF_BLOCKS);

    // for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
    //     g.blocks[i].update(deltaTime, nullptr, nullptr, 0, 
    //         nullptr, 0);

    // for (int i = 0; i < NUMBER_OF_TILES; i++) 
    //     g.tiles[i].update(deltaTime, nullptr, nullptr, 0, 
    //         nullptr, 0);

    // if (g.xochitl->getPosition().y > END_GAME_THRESHOLD) 
    //     gAppStatus = TERMINATED;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

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

#ifdef DEBUG
    if (g.LunarLander)
    {
        DrawText(gDebugMessage, 20, 70, 18, YELLOW);
    }
#endif

    // g.xochitl->render();
    // g.ghost->render();

    // for (int i = 0; i < NUMBER_OF_TILES;  i++) g.tiles[i].render();
    // for (int i = 0; i < NUMBER_OF_BLOCKS; i++) g.blocks[i].render();
    switch (g.gameStatus)
    {
        case GAME_START:
            DrawText("Press Enter to Start", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, WHITE);
            break;
        case GAME_RUNNING:
            g.LunarLander->render();
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
    }

    EndDrawing();
}

void shutdown() 
{
    // delete   g.xochitl;
    // delete[] g.tiles;
    // delete[] g.blocks;
    // delete   g.ghost;

    delete g.LunarLander;

    StopMusicStream(g.bgm);
    UnloadMusicStream(g.bgm);
    UnloadSound(g.burstSound);

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
