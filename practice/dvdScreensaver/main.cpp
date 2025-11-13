#include "raylib.h"
// #include "lib/vector_ops.h"
// #include "lib/pid_controller.h"
#include "lib/helper.h"
#include "lib/Entity.h"


// Global Constants
struct Constants {
    const char *TITLE = "Hello raylib!";
    int SCREEN_WIDTH = static_cast<int>(800 * 1.5f);
    int SCREEN_HEIGHT = static_cast<int>(450 * 1.5f);
    constexpr static int FPS = 60;
    constexpr static float FIXED_TIMESTEP = 1.0f / 60.0f;
};

// game state
struct GameState {
    Entity dvd;
    Vector2 dvdVelocity = { 200.0f, 160.0f };
    float PreviousTicks = 0.0f;
    float TimeAccumulator = 0.0f;
    float DeltaTime = 0.0f;
};


// Global Variables
AppStatus gAppStatus   = RUNNING;
Constants c;
GameState g;

// Function Declarations
void initialise();
void processInput();
void update();
void updateDvd(float deltaTime);
Color randomDvdColor();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(c.SCREEN_WIDTH, c.SCREEN_HEIGHT, c.TITLE);
    SetWindowState(FLAG_FULLSCREEN_MODE);
    int monitor = GetCurrentMonitor();
    c.SCREEN_WIDTH = GetMonitorWidth(monitor);
    c.SCREEN_HEIGHT = GetMonitorHeight(monitor);
    LOG_INFO(TextFormat("Window width: %d, height: %d", c.SCREEN_WIDTH, c.SCREEN_HEIGHT));

    SetTargetFPS(c.FPS);

    g.dvd = Entity({c.SCREEN_WIDTH / 2.0f, c.SCREEN_HEIGHT / 2.0f}, {453.0f / 2.0f, 204.0f / 2.0f}, "asserts/dvd.png", NPC); // unnecessary though
    g.dvd.setTint(randomDvdColor());

    auto randomDirection = []() {
        return GetRandomValue(0, 1) == 0 ? -1.0f : 1.0f;
    };
    g.dvdVelocity = { 200.0f * randomDirection(), 160.0f * randomDirection() };
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {
    LOG_DEBUG("Update");
    float ticks = (float) GetTime();
    float deltaTime = ticks - g.PreviousTicks;
    g.PreviousTicks  = ticks;

    g.TimeAccumulator += deltaTime;

    while (g.TimeAccumulator >= c.FIXED_TIMESTEP) {
        g.TimeAccumulator -= c.FIXED_TIMESTEP;
        LOG_DEBUG("Fixed timestep update");
        updateDvd(c.FIXED_TIMESTEP);
    }

    g.DeltaTime = g.TimeAccumulator;
}

void render()
{
    BeginDrawing();

    ClearBackground(BLACK);

    g.dvd.render();
    if (debug) g.dvd.displayCollider();

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow(); // Close window and OpenGL context
}

void updateDvd(float deltaTime)
{
    Vector2 position = g.dvd.getPosition();
    position.x += g.dvdVelocity.x * deltaTime;
    position.y += g.dvdVelocity.y * deltaTime;

    float halfWidth  = g.dvd.getScale().x / 2.0f;
    float halfHeight = g.dvd.getScale().y / 2.0f;

    float minX = halfWidth;
    float maxX = c.SCREEN_WIDTH - halfWidth;
    float minY = halfHeight;
    float maxY = c.SCREEN_HEIGHT - halfHeight;

    bool bounced = false;

    if (position.x <= minX) {
        position.x = minX;
        g.dvdVelocity.x *= -1.0f;
        bounced = true;
    } else if (position.x >= maxX) {
        position.x = maxX;
        g.dvdVelocity.x *= -1.0f;
        bounced = true;
    }

    if (position.y <= minY) {
        position.y = minY;
        g.dvdVelocity.y *= -1.0f;
        bounced = true;
    } else if (position.y >= maxY) {
        position.y = maxY;
        g.dvdVelocity.y *= -1.0f;
        bounced = true;
    }

    if (bounced) {
        g.dvd.setTint(randomDvdColor());
    }

    g.dvd.setPosition(position);
}

Color randomDvdColor()
{
    float hue = static_cast<float>(GetRandomValue(0, 359));
    float saturation = 0.8f;
    float value = 0.95f;
    return ColorFromHSV(hue, saturation, value);
}

int main(int argc, char *argv[])
{
    init_log_level(argc, argv);
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
