#include "CS3113/Entity.h"

struct GameState
{
    Entity *xochitl;
    Entity *tiles;
    Entity *blocks;
    Entity *ghost;

    Music bgm;
    Sound jumpSound;
};

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#C0897E";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 6, 8 };

constexpr int   NUMBER_OF_TILES         = 20,
                NUMBER_OF_BLOCKS        = 3;
constexpr float TILE_DIMENSION          = 50.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 981.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

GameState g;

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

    g.bgm = LoadMusicStream("assets/game/04 - Silent Forest.wav");
    SetMusicVolume(g.bgm, 0.33f);
    PlayMusicStream(g.bgm);

    g.jumpSound = LoadSound("assets/game/Dirt Jump.wav");

    /*
        ----------- PROTAGONIST -----------
    */
    std::map<Direction, std::vector<int>> xochitlAnimationAtlas = {
        {DOWN,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
        {LEFT,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
        {UP,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
        {RIGHT, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    };

    float sizeRatio  = 48.0f / 64.0f;

    // Assets from @see https://sscary.itch.io/the-adventurer-female
    g.xochitl = new Entity(
        {ORIGIN.x - 300.0f, ORIGIN.y - 200.0f}, // position
        {250.0f * sizeRatio, 250.0f},           // scale
        "assets/game/walk.png",                 // texture file address
        ATLAS,                                  // single image or atlas?
        ATLAS_DIMENSIONS,                       // atlas dimensions
        xochitlAnimationAtlas,                  // actual atlas
        PLAYER                                  // entity type
    );

    g.xochitl->setJumpingPower(450.0f);
    g.xochitl->setColliderDimensions({
        g.xochitl->getScale().x / 3.0f,
        g.xochitl->getScale().y / 3.0f
    });
    g.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- TILES -----------
    */
    g.tiles = new Entity[NUMBER_OF_TILES];

    // Compute the left‑most x coordinate so that the entire row is centred
    float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;

    for (int i = 0; i < NUMBER_OF_TILES; i++) 
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        g.tiles[i].setTexture("assets/game/tile_0000.png");
        g.tiles[i].setEntityType(PLATFORM);
        g.tiles[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        g.tiles[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
        g.tiles[i].setPosition({
            leftMostX + i * TILE_DIMENSION, 
            ORIGIN.y + TILE_DIMENSION
        });
    }

    /*
        ----------- BLOCKS -----------
    */
    g.blocks = new Entity[NUMBER_OF_BLOCKS];

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        g.blocks[i].setTexture("assets/game/tile_0061.png");
        g.blocks[i].setEntityType(BLOCK);
        g.blocks[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        g.blocks[i].setColliderDimensions(
            {TILE_DIMENSION, TILE_DIMENSION});
    }

    g.blocks[0].setPosition(
        {ORIGIN.x - TILE_DIMENSION * 3, ORIGIN.y - TILE_DIMENSION * 2.5f});
    g.blocks[1].setPosition(
        {ORIGIN.x, ORIGIN.y - TILE_DIMENSION * 2.5f});
    g.blocks[2].setPosition(
        {ORIGIN.x + TILE_DIMENSION * 3, ORIGIN.y - TILE_DIMENSION * 2.5f});


    /*
        ----------- GHOST -----------
    */
    std::map<Direction, std::vector<int>> ghostAnimationAtlas = {
        {LEFT,  { 1, 9, 17, 25 }},
        {RIGHT, { 0, 8, 16, 24 }},
    };

    // @see dyru.itch.io/pixel-ghost-template
    g.ghost = new Entity(
        {ORIGIN.x + 300.0f, ORIGIN.y - 200.0f}, // position
        {100.0f, 100.0f},                       // scale
        "assets/game/gosth.png",                // texture file address
        ATLAS,                                  // single image or atlas?
        ATLAS_DIMENSIONS,                       // atlas dimensions
        ghostAnimationAtlas,                    // actual atlas
        NPC                                     // entity type
    );

    g.ghost->setAIType(FOLLOWER);
    g.ghost->setAIState(IDLE);
    g.ghost->setSpeed(Entity::DEFAULT_SPEED * 0.50f);

    g.ghost->setColliderDimensions({
        g.ghost->getScale().x / 2.0f,
        g.ghost->getScale().y
    });

    g.ghost->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    g.ghost->setDirection(LEFT);
    g.ghost->render(); // calling render once at the beginning to switch ghost's direction

    SetTargetFPS(FPS);
}

void processInput() 
{
    g.xochitl->resetMovement();
    g.ghost->resetMovement();

    if      (IsKeyDown(KEY_A)) g.xochitl->moveLeft();
    else if (IsKeyDown(KEY_D)) g.xochitl->moveRight();

    if (IsKeyPressed(KEY_W) && g.xochitl->isCollidingBottom())
    {
        g.xochitl->jump();
        PlaySound(g.jumpSound);
    }

    // to avoid faster diagonal speed
    if (GetLength(g.xochitl->getMovement()) > 1.0f) 
        g.xochitl->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        UpdateMusicStream(g.bgm);

        g.xochitl->update(FIXED_TIMESTEP, nullptr, g.tiles, 
            NUMBER_OF_TILES, g.blocks, NUMBER_OF_BLOCKS);

        g.ghost->update(FIXED_TIMESTEP, g.xochitl, g.tiles, 
            NUMBER_OF_TILES, g.blocks, NUMBER_OF_BLOCKS);

        for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
            g.blocks[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0, 
                nullptr, 0);

        for (int i = 0; i < NUMBER_OF_TILES; i++) 
            g.tiles[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0, 
                nullptr, 0);

        deltaTime -= FIXED_TIMESTEP;
    }

    if (g.xochitl->getPosition().y > END_GAME_THRESHOLD) 
        gAppStatus = TERMINATED;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    g.xochitl->render();
    g.ghost->render();

    for (int i = 0; i < NUMBER_OF_TILES;  i++) g.tiles[i].render();
    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) g.blocks[i].render();

    EndDrawing();
}

void shutdown() 
{
    delete   g.xochitl;
    delete[] g.tiles;
    delete[] g.blocks;
    delete   g.ghost;

    UnloadMusicStream(g.bgm);
    UnloadSound(g.jumpSound);

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