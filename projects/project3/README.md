# Project 3 - Lunar Lander

[instruction](instruction.md)

A Raylib-based Lunar Lander simulation for CS-3113 Project 3 that satisfies all assignment requirements including gravity-driven physics, acceleration-based movement, mission success/failure states, fuel mechanics, and sprite sheet animation. The game features realistic space physics with translation and rotation stabilizers, limited engine ignitions, procedurally generated lunar terrain, and a moving ISS obstacle.

See `instruction.md` for the original rubric and grading guidelines.

## Features

- **Realistic Physics**: 1/6 Earth gravity simulation with acceleration-based thrust and RCS controls
- **Fuel Management**: Limited fuel supply with real-time consumption tracking and UI display
- **Engine System**: Main engine with throttle control, smooth startup/shutdown transitions, and limited ignitions
- **Stabilization Systems**: Optional translation and rotation stabilizers for precise maneuvering
- **Procedural Terrain**: Dynamically generated lunar surface using layered sine wave noise
- **Moving Obstacle**: International Space Station (ISS) moving platform that causes mission failure on collision
- **Sprite Sheet Animation**: Multi-state flame animation (start, loop, end) synchronized with engine throttle
- **Advanced Controls**: WASD throttle adjustment, IJKL RCS translation, AD rotation, stabilizer toggles
- **Dynamic Camera**: Smooth camera tracking with zoom, map view, and parallax scrolling background
- **Win/Lose Conditions**: Safe landing requires both feet down, low speed (<25 units), and near-upright angle (<15°)
- **Leaderboard Integration**: Anonymous telemetry submission for landing attempts (fail-open design)

## How This Project Was Implemented

### Physics Architecture

- Extend the `Entity` class to create a `Lander` class with advanced physics properties:
  - Mass management: dry mass + fuel mass (dynamic)
  - Moment of inertia for rotational physics
  - Force and torque accumulation with delta-time integration
  - Separate acceleration tracking for gravity, thrust, and control inputs

### Control Systems

- **Main Engine**: State machine (Off → Starting → Running → Stopping) with smooth throttle transitions

  - Consumes fuel based on throttle setting (exponential consumption curve)
  - Limited to 3 ignitions throughout the mission
  - Visual feedback via animated flame sprite sheets
  - Audio feedback with dynamic volume based on throttle
- **RCS Thrusters**: Four-directional translation control (IJKL keys)

  - Applies forces in lander-relative coordinates
  - Consumes fuel at a constant rate during use
  - Automatically disabled when fuel depleted
- **Stabilization System**: Optional PID-like controllers for drift compensation

  - Translation stabilizer: counteracts linear velocity when no manual input
  - Rotation stabilizer: counteracts angular velocity when no manual input
  - Override mechanism: temporarily disables stabilizers during manual input

### Terrain & Collision

- Procedurally generated terrain using dual-frequency sine waves for natural variation
- Tile-based rendering with 8×6 texture atlas for visual variety
- Collision detection using two "landing feet" at the lander's base
- Safe landing requires both feet in contact, low speed, and near-vertical orientation

### Game States

- `GAME_START`: Initialize lander position, reset fuel and ignitions
- `GAME_RUNNING`: Active physics simulation and collision detection
- `GAME_PAUSED`: Freeze simulation, retain state
- `GAME_WON`: Both feet down, speed < 25, angle < 15°
- `GAME_OVER`: Crash (high speed/bad angle), ISS collision, or fall below threshold

## Technical Details

```cpp
// Lander Class (extends Entity)
class Lander : public Entity {
public:
    // Stabilizer controls
    void toggleTranslationStabiliser();
    void toggleRotationStabiliser();
    void resetControllers();

    // Main engine controls
    void toggleMainEngine();                     // Start/stop with ignition limit
    void adjustMainEngineThrottle(float delta);  // W/S keys for fine control
    void setMainEngineThrottle(float throttle);  // Z/X for min/max

    // RCS controls
    void applyTranslationStabilisation(float deltaTime);
    void applyRotationStabilisation(float deltaTime);

    // Fuel system
    float getFuel() const;
    bool consumeRCSFuel(float amount);
    int getIgnitionsRemaining() const;

private:
    enum class MainEngineState { Off, Starting, Running, Stopping };

    MainEngineState mEngineState;
    float mFuel;
    float mDryMass;
    float mTargetThrottle;
    float mCurrentThrottle;
    int mIgnitionsRemaining;
    bool mTranslationStabiliserEnabled;
    bool mRotationStabiliserEnabled;
};

// Collision detection
struct TerrainContactInfo {
    bool leftContact;
    bool rightContact;
    float impactSpeed;
};

TerrainContactInfo resolveTerrainCollision(Lander *lander) {
    // Calculate two landing feet positions
    Vector2 leftFoot = position + rotatePoint({-halfWidth * 0.65f, halfHeight}, angle);
    Vector2 rightFoot = position + rotatePoint({halfWidth * 0.65f, halfHeight}, angle);

    // Check terrain height at each foot
    // Resolve penetration and detect contact
    // Return contact info for landing evaluation
}

// Flame animation states
enum FlameState { FLAME_OFF, FLAME_START, FLAME_LOOP, FLAME_END };
void updateFlameAnimation(float deltaTime, bool engineFiring);
```

## Build & Run

This project uses Raylib and requires a C++17-capable compiler.

### macOS

1. Install Raylib:

   ```bash
   brew install raylib
   ```
2. Build the project:

   ```bash
   make
   ```
3. Run the game:

   ```bash
   make run
   ```
4. Clean build artifacts:

   ```bash
   make clean
   ```

### Other Platforms

Ensure Raylib is installed and accessible via `pkg-config`. Adjust the Makefile if needed for your platform's compiler and library paths.

## Controls

### Flight Controls

- **Space**: Toggle main engine (limited to 3 ignitions)
- **W / S**: Increase / decrease throttle
- **Z / X**: Set throttle to maximum / minimum
- **A / D**: Rotate left / right (consumes RCS fuel)
- **I / J / K / L**: RCS translation (forward / left / backward / right, consumes fuel)

### System Toggles

- **G**: Toggle translation stabilizer (drift compensation)
- **T**: Toggle rotation stabilizer (spin compensation)

### Camera & Interface

- **Mouse Wheel**: Zoom in / out
- **M**: Toggle map view (wide-angle overview)
- **P**: Pause game
- **Q**: Quit
- **Enter**: Start mission / restart after game over

## Key Configuration Constants (in `main.cpp`)

```cpp
constexpr float ACCELERATION_OF_GRAVITY = 16.35f; // 1/6 of gravity on Earth
constexpr float END_GAME_THRESHOLD      = -800.0f;
      
constexpr float   TRANSLATIONAL_THRUST = 50.0f;
constexpr float   ROTATIONAL_THRUST = 50.0f;
constexpr float   TRANSLATIONAL_STABILISER_THRUST = 10.0f;
constexpr float   ROTATIONAL_STABILISER_THRUST = 10.0f;
constexpr float   TRANSLATIONAL_STABILISER_MINIMUM_SPEED = 0.25f;
constexpr float   ROTATIONAL_STABILISER_MINIMUM_SPEED = 0.25f;
constexpr float   LUNAR_LANDER_MASS = 1.0f;
constexpr float   LUNAR_LANDER_MOMENT = 1.0f;
constexpr Vector2 LUNAR_LANDER_INIT_POSITION = {ORIGIN.x, ORIGIN.y - 600.0f};
constexpr Vector2 LUNAR_LANDER_INIT_VELOCITY = {200.0f, 00.0f};
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
constexpr float   RCS_FUEL_BURN_RATE = 0.5f;
constexpr float   BACKGROUND_PARALLAX_FACTOR = 0.25f;
constexpr float   BACKGROUND_VELOCITY_OFFSET_SCALE = 0.08f;
constexpr float   CAMERA_ZOOM_MIN = 0.35f;
constexpr float   CAMERA_ZOOM_MAX = 4.0f;
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
constexpr float   CONTACT_EPSILON = 16.0f;
constexpr float   LANDER_FOOT_SPAN_RATIO = 0.65f;
constexpr float   CAMERA_PAN_LERP_SPEED = 0.1f;
constexpr float   MAX_SAFE_LANDING_SPEED = 25.0f;
constexpr float   INPUT_EPSILON = 0.01f;
constexpr float   DEBUG_DISPLAY_INTERVAL = 0.15f;

// Leaderboard Configuration
constexpr bool    LEADERBOARD_ENABLED = true;
// https://api.lishuyu.top/project/lunarlanderleaderboard/
// This submission is fully anonymous, no personal information is collected.
constexpr char    LEADERBOARD_URL[] = "https://api.lishuyu.top/api/project/lunarlanderleaderboard/"; 
constexpr float   LEADERBOARD_TIMEOUT = 2.0f;

// landing
constexpr float MAX_SAFE_LANDING_ANGLE = 15.0f; // degrees
```

Adjust these values to tune difficulty, fuel constraints, or physics behavior.

## Grading Criteria Alignment

### ✅ Requirement 1: Player Falls With Gravity (25%)

The lander experiences constant downward acceleration simulating 1/6 Earth gravity (16.35 units/s²), applied every frame via `setAcceleration({0.0f, ACCELERATION_OF_GRAVITY})`. This creates the characteristic slow fall of lunar gravity.

```cpp
constexpr float ACCELERATION_OF_GRAVITY = 16.35f;  // 1/6 Earth gravity
g.LunarLander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
```

### ✅ Requirement 2: Moving With Acceleration (25%)

All movement modifies acceleration via force application, never directly setting velocity:

- Main engine applies thrust force: `applyForce(thrustDirection * magnitude)` 
- RCS thrusters apply directional forces: `applyForce(translationalThrust)` 
- Rotation uses torque: `applyTorque(rotationalTorque)` 
- Stabilizers apply corrective forces, not velocity manipulation 

The Entity physics system integrates forces → acceleration → velocity → position using delta time, creating realistic drift when control inputs cease.

### ✅ Requirement 3: Mission Failed / Mission Accomplished (25%)


**Win Condition** : Evaluated in `evaluateLandingOutcome()`

- Both landing feet must contact terrain
- Speed must be < 25 units/s
- Angle must be within ±15° of vertical
- Triggers `GAME_WON` state with success message

**Lose Conditions**:

1. **Crash landing**: High speed (≥25) or bad angle (≥15°) → `GAME_OVER`
2. **ISS collision**: Hit moving platform → `GAME_OVER` 
3. **Fall below map**: Position.y < -800 → `GAME_OVER` 

**Moving Platform**: International Space Station moves horizontally at 60 units/s across the top of the screen, wrapping around when off-screen. Collision with ISS immediately ends the game .

### ✅ Requirement 4: Fuel Mechanic (25%)

**Fuel System**:

- Initial capacity: 200 units
- Main engine consumption: 0.5-5.0 units/s based on throttle 
- RCS consumption: 0.5 units/s when any RCS key pressed 
- Fuel affects lander mass dynamically: `totalMass = dryMass + (fuel * 0.02)` 

**Controls Lockout**: When fuel depletes:

- Main engine cannot start 
- RCS thrusters disabled 
- Engine auto-shuts down if running when fuel exhausted 

**UI Display** : Shows fuel amount, percentage, throttle setting, and remaining ignitions in real-time.

### 🌟 Extra Credit: Sprite Sheet Animation (Bonus)

Multi-state flame animation system using three sprite sheets:

- **Start sequence** (`burning_start_1x4.png`): 4 frames for ignition
- **Loop sequence** (`burning_loop_1x8.png`): 8 frames for sustained burn
- **End sequence** (`burning_end_1x5.png`): 5 frames for shutdown

State machine transitions: `FLAME_OFF → FLAME_START → FLAME_LOOP → FLAME_END → FLAME_OFF`

Dynamic scaling and frame rate based on throttle:

```cpp
float throttleNormalised = getMainEngineAppliedThrottleNormalised() * getMainEngineOutputLevel();
float frameSpeed = 8.0f + throttleNormalised * 8.0f;  // 8-16 FPS
flameScale.y = landerScale.y * (0.9f + 0.4f * throttleNormalised);  // Grows with throttle
```

## Submission Checklist

- ✅ Delta time used for all physics calculations (`main.cpp:1042-1047`)
- ✅ Entity class utilized (`Lander` extends `Entity`, `main.cpp:225-795`)
- ✅ Fixed time step clamping implemented (`deltaTime > 0.02f ? 0.02f : deltaTime`)
- ✅ Header comment block present with academic honesty pledge (`main.cpp:1-9`)
- ✅ Assignment title correctly states "Lunar Lander" (not "Pong Clone")
- ✅ GitHub repository with public access
- ✅ All required assets included in `assets/` directory
- ✅ No functionality used outside course curriculum (Raylib only)
- ✅ All four requirements satisfied + extra credit

## Additional Features (Beyond Requirements)

- **Dual Stabilization System**: Independent translation and rotation stabilizers with override logic
- **Throttle Control**: 15%-100% range with smooth transitions and variable fuel efficiency
- **Limited Ignitions**: Engine can only be started 3 times (realistic mission constraint)
- **Engine State Machine**: Realistic startup/shutdown sequences with visual/audio feedback
- **Procedural Terrain**: Infinite scrolling lunar surface with tiled texture atlas
- **Advanced Camera**: Smooth tracking, zoom control, and map view toggle
- **Parallax Scrolling**: Background offset based on camera position and lander velocity
- **Comprehensive Telemetry**: Real-time HUD showing position, velocity, angle, fuel, contacts
- **Leaderboard Integration**: Anonymous submission of landing attempts with full telemetry
- **Visual Feedback**: Color-coded UI elements (green/orange/red for speed, angle, contacts)
- **Audio System**: Background music and dynamic engine sound with throttle-based volume

## Screenshots

**Game Start**
![Game Start](<assets/screenshots/Screenshot 2025-10-26 at 20.54.02.png>)

**In Flight**
![In Flight](<assets/screenshots/Screenshot 2025-10-26 at 20.53.55.png>)

**Mission Accomplished**
![Mission Accomplished](<assets/screenshots/Screenshot 2025-10-26 at 20.54.33.png>)

**Mission Failed**
![Mission Failed](<assets/screenshots/Screenshot 2025-10-26 at 20.53.59.png>)
---

## Requirements Satisfaction Summary

| Requirement                              | Satisfied | Evidence                                                                                                                                                            |
| ---------------------------------------- | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **1. Gravity Physics**             | ✅ YES    | Constant 16.35 units/s² downward acceleration                                                                                  |
| **2. Acceleration-Based Movement** | ✅ YES    | All controls apply forces/torques, never directly modify velocity                                           |
| **3. Win/Lose Conditions**         | ✅ YES    | Safe landing requires both feet down + low speed + upright angle . ISS moving platform causes failure on collision |
| **4. Fuel Mechanic**               | ✅ YES    | 200-unit capacity, throttle-based consumption, UI display, controls lockout when depleted      |
| **Extra Credit: Animation**        | ✅ YES    | Three-stage sprite sheet system (start/loop/end) with throttle-responsive scaling and frame rate                                           |

