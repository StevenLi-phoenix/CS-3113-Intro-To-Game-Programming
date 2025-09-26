/**
* Author: Steven Li
* Assignment: Pong Game
* Date due: 2025-10-10, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
* Author: Steven Li
* Time: 2025/09/26 18:14PM
* Assignment link: TBD
* File UUID: TBD
*/

#include "CS3113/cs3113.h"
#include <math.h>
#include <iostream>
using namespace std;

// Global Constants
constexpr int SCREEN_WIDTH = 800 * 1.5f,
              SCREEN_HEIGHT = 450 * 1.5f,
              FPS = 60;

constexpr int PADDLE_WIDTH = 10;
constexpr int PADDLE_HEIGHT = 100;
constexpr int PADDLE_SPEED = 100;
constexpr int PADDLE_INIT_POS_SCREEN_MARGIN = SCREEN_WIDTH / 25;

// Global Variables
AppStatus gAppStatus = RUNNING;
Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
Texture2D gbackground;
float deltaTime = 0.0f;
float gPreviousTicks = 0.0f;

// GameObject class
/**
* @brief GameObject class
*
* @param position The position of the game object
* @param velocity The velocity of the game object
* @param acceleration The acceleration of the game object
* @param mass The mass of the game object
* @param scale The scale of the game object
* @param angle The angle of the game object
* @param texture The texture of the game object, should use in draw() override in child classes
*/
class GameObject
{
public:
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float mass;
    Vector2 scale;
    float angle;
    Color color;

    GameObject(Vector2 position, Vector2 velocity, Vector2 acceleration, float mass, Vector2 scale, float angle, Color color): position(position), velocity(velocity), acceleration(acceleration), mass(mass), scale(scale), angle(angle), color(color)
    {
    }
    virtual ~GameObject() = default;
    virtual void draw() = 0; // pure virtual, must be overridden in child classes
    void update(float deltaTime)
    {
        velocity.x += acceleration.x * deltaTime;
        velocity.y += acceleration.y * deltaTime;
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
        acceleration.x = 0.0f;
        acceleration.y = 0.0f;
    }
    void applyForce(Vector2 force)
    {
        acceleration.x += force.x / mass;
        acceleration.y += force.y / mass;
    }
};

class Ball : public GameObject
{
public:
    Ball(Vector2 position, Vector2 velocity, Vector2 acceleration, float mass, Vector2 scale, float angle, Color color)
        : GameObject(position, velocity, acceleration, mass, scale, angle, color)
    {
    }
    void draw() override
    {
        DrawCircle(this->position.x, this->position.y, this->scale.x, this->color);
    }
};

class Paddle : public GameObject
{
public:
    Paddle(Vector2 position, Vector2 velocity, Vector2 acceleration, float mass, Vector2 scale, float angle, Color color)
        : GameObject(position, velocity, acceleration, mass, scale, angle, color)
    {
    }
    void draw() override
    {
        DrawRectangle(
            this->position.x - this->scale.x / 2,
            this->position.y - this->scale.y / 2,
            this->scale.x,
            this->scale.y,
            color
        );
    }
};

Ball gBall({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, {0, 0}, {0, 0}, 1, {10, 10}, 0, GREEN);
Paddle gPaddle({PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, {0, 0}, {0, 0}, 1, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, RED);
Paddle gPaddle2({SCREEN_WIDTH - PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, {0, 0}, {0, 0}, 1, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, BLUE);

// helper functions
void updateDeltaTime();
bool isCollidingBox(GameObject* object1, GameObject* object2);
// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 2: Pong Game");

    gbackground = LoadTexture("assets/pong_background.png");

    SetTargetFPS(FPS);

    gBall.velocity.x = 100;
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyDown(KEY_ESCAPE)) gAppStatus = TERMINATED;
}

void update() {
    updateDeltaTime();

    gBall.update(deltaTime);
    gPaddle.update(deltaTime);
    gPaddle2.update(deltaTime);
}

void render()
{
    BeginDrawing();
    // Draw the background
    ClearBackground(GRAY);
    DrawTexturePro(
        gbackground,
        {0, 0, (float)gbackground.width, (float)gbackground.height},
        {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT},
        {0, 0},
        0,
        WHITE
    );
    gBall.draw();
    gPaddle.draw();
    gPaddle2.draw();

    DrawFPS(10, 10);

    EndDrawing();
}

void shutdown()
{
    CloseWindow(); // Close window and OpenGL context
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

// helper function definitions
void updateDeltaTime()
{
    float ticks = (float) GetTime();
    deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
}
/**
* @brief Checks if two boxes are colliding
*
* @param object1 The first object
* @param object2 The second object
* @return true if the objects outer boxes are colliding, false otherwise
*/
bool isCollidingBox(GameObject* object1, GameObject* object2)
{
    float xDistance = fabs(object1->position.x - object2->position.x) - ((object1->scale.x + object2->scale.x) / 2.0f);
    float yDistance = fabs(object1->position.y - object2->position.y) - ((object1->scale.y + object2->scale.y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}
