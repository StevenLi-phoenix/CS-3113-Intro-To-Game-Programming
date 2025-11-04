#include "level_example.h"
#include "witch.h"

// ===========================
// CONSTRUCTORS & DESTRUCTOR
// ===========================

// Default constructor - initializes with zero origin and no background
LevelExample::LevelExample() : Scene{{0.0f}, nullptr} {}

// Constructor with custom origin and background color
LevelExample::LevelExample(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

// Destructor - ensures proper cleanup
LevelExample::~LevelExample() { shutdown(); }

// ===========================
// INITIALIZATION
// ===========================

void LevelExample::initialise()
{
    // --- SCENE CONTROL ---
    mGameState.nextSceneID = 0;  // 0 = stay in this level, change for level transitions

    // --- AUDIO SETUP ---
    // Load background music
    // mGameState.bgm = LoadMusicStream("assets/audio/background.wav");
    // SetMusicVolume(mGameState.bgm, 0.33f);
    // PlayMusicStream(mGameState.bgm);

    // Load sound effects
    // mGameState.jumpSound = LoadSound("assets/audio/jump.wav");

    // --- MAP INITIALIZATION ---
    // Create the level tilemap
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,   // Map grid dimensions (columns × rows)
        (unsigned int *)mLevelData,  // Tile data array
        "assets/world_tileset.png",  // Tileset texture path (16x16 atlas)
        TILE_DIMENSION,              // Size of each tile in pixels
        16, 16,                      // Tileset dimensions (16 columns × 16 rows)
        mOrigin                      // In-game origin point (screen center)
    );

    // --- PLAYER INITIALIZATION ---
    // Create the witch player character
    // Note: Using Witch class which should have proper animation setup
    Witch *witch = new Witch();
    
    // Configure witch properties
    witch->setPosition({mOrigin.x - 300.0f, mOrigin.y - 200.0f});  // Starting position
    witch->setJumpingPower(550.0f);                                // Jump strength
    witch->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});       // Apply gravity
    
    // Set collision box (adjust based on sprite size)
    witch->setColliderDimensions({
        witch->getScale().x / 3.5f,  // Width of collision box
        witch->getScale().y / 3.0f   // Height of collision box
    });

    // Store witch as the player entity
    // TODO: Add Witch* to GameState or use Entity* with proper setup
    // For now, you'll need to modify Scene.h GameState to include:
    // Witch *witch; or Entity *player;
    
    // --- CAMERA INITIALIZATION ---
    // Set up camera to follow the player
    mGameState.camera = {0};                      // Zero-initialize camera
    mGameState.camera.target = witch->getPosition(); // Follow witch
    mGameState.camera.offset = mOrigin;           // Center on screen
    mGameState.camera.rotation = 0.0f;            // No rotation
    mGameState.camera.zoom = 1.0f;                // Default zoom level
}

// ===========================
// UPDATE LOOP
// ===========================

void LevelExample::update(float deltaTime)
{
    // --- UPDATE AUDIO ---
    // UpdateMusicStream(mGameState.bgm);

    // --- UPDATE PLAYER ---
    // TODO: Get witch reference from GameState
    // witch->update(
    //     deltaTime,           // Frame time delta
    //     nullptr,             // Player reference (for AI enemies)
    //     mGameState.map,      // Map for collision detection
    //     nullptr,             // Array of collidable entities
    //     0                    // Count of collidable entities
    // );

    // --- CAMERA TRACKING ---
    // Update camera to follow player (only horizontal tracking)
    // Vector2 currentPlayerPosition = {witch->getPosition().x, mOrigin.y};
    // panCamera(&mGameState.camera, &currentPlayerPosition);

    // --- LEVEL TRANSITIONS / DEATH ---
    // Check if player fell off the map (Y > threshold)
    // if (witch->getPosition().y > END_GAME_THRESHOLD) {
    //     mGameState.nextSceneID = 1;  // Transition to next scene/death scene
    // }
}

// ===========================
// RENDERING
// ===========================

void LevelExample::render()
{
    // --- CLEAR SCREEN ---
    ClearBackground(ColorFromHex(mBGColourHexCode));  // Set background color

    // --- RENDER GAME OBJECTS ---
    // Order matters: render back to front
    mGameState.map->render();           // Render tilemap first (background)
    // witch->render();                 // TODO: Render witch on top of map

    // --- DEBUG RENDERING (optional) ---
    // witch->displayCollider();        // Show collision box for debugging
}

// ===========================
// CLEANUP
// ===========================

void LevelExample::shutdown()
{
    // --- DELETE GAME OBJECTS ---
    // Prevent memory leaks by deleting dynamically allocated objects
    // delete witch;              // TODO: Delete witch
    delete mGameState.map;        // Delete tilemap

    // --- UNLOAD AUDIO ---
    // UnloadMusicStream(mGameState.bgm);
    // UnloadSound(mGameState.jumpSound);
}
