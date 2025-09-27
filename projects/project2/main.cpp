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
#include <vector>
#include <string>
#include <iostream>
using namespace std;

enum PaddleStatus { PLAYER_CONTROLLED, AI_CONTROLLED };

// Global Constants
constexpr int SCREEN_WIDTH = 800 * 1.5f,
              SCREEN_HEIGHT = 450 * 1.5f,
              FPS = 60;

constexpr int PADDLE_WIDTH = 10;
constexpr int PADDLE_HEIGHT = 100;
constexpr int PADDLE_SPEED = 200;
constexpr int PADDLE_INIT_POS_SCREEN_MARGIN = SCREEN_WIDTH / 25;
constexpr int PADDLE_MASS = 1;
constexpr int BALL_SPEED = 200;
constexpr int BALL_SIZE = 100;
constexpr int BALL_MASS = 1;
constexpr bool ENABLE_WIND_RESISTANCE = true;
constexpr bool ENABLE_GROUND_FRICTION = true;
constexpr float WIND_FRICTION = 0.001f;
constexpr float GROUND_FRICTION = 0.1f;
constexpr float G = 9.8f;
constexpr int BALL_COUNT = 10000;

// Forward declarations
class GameObject;
class Ball;
class Paddle;

// Global Variables
AppStatus gAppStatus = RUNNING;
Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
Texture2D gbackground;
Texture2D gStackedDigitsTexture;
float deltaTime = 0.0f;
float gPreviousTicks = 0.0f;
PaddleStatus gPaddleStatus = PLAYER_CONTROLLED;
vector<Ball> gBalls;
vector<Paddle> gPaddles;
int gLeftScore = 0;
int gRightScore = 0;
int gBallCount = BALL_COUNT;
// FPS tracking variables
static float fps = 0.0f;
static float fpsSum = 0.0f;
int gFrameCount = 0;
static const int AVG_WINDOW = 60;
static float fpsWindow[AVG_WINDOW] = {0};
static int windowIndex = 0;


// helper functions
void updateDeltaTime();
bool isCollidingBox(GameObject* object1, GameObject* object2);

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// GameObject class
class GameObject
{
public:
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 collisionBox;
    float mass;
    Vector2 scale;
    float angle;
    float speed;
    Color color;
    bool windResistance = ENABLE_WIND_RESISTANCE;
    bool groundFriction = ENABLE_GROUND_FRICTION;
    GameObject(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color): position(position), mass(mass), scale(scale), angle(angle), speed(speed), color(color), velocity({0, 0}), acceleration({0, 0})
    {
        this->velocity.x = speed * cos(angle * PI / 180);
        this->velocity.y = speed * sin(angle * PI / 180);
        this->collisionBox = {scale.x, scale.y};
    }
    virtual ~GameObject() = default;
    virtual void draw() = 0; // pure virtual, must be overridden in child classes
    void update(float deltaTime)
    {
        if (windResistance) applyForce(calculateWindResistanceForce());
        if (groundFriction) applyForce(calculateGroundFrictionForce());
        velocity.x += acceleration.x * deltaTime;
        velocity.y += acceleration.y * deltaTime;
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
        acceleration.x = 0.0f;
        acceleration.y = 0.0f;
    }
    void applyForce(Vector2 force)
    {
        if (abs(force.x) < 0.0001f) force.x = 0.0f;
        if (abs(force.y) < 0.0001f) force.y = 0.0f;
        acceleration.x += force.x / mass;
        acceleration.y += force.y / mass;
    }
    float getSpeed()
    {
        return sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    }
    float getSpeedSquare()
    {
        return velocity.x * velocity.x + velocity.y * velocity.y;
    }
    void setSpeed(float speed)
    {
        this->speed = speed;
        this->velocity.x = speed * cos(angle);
        this->velocity.y = speed * sin(angle);
    }
    float getAngle()
    {
        return atan2(velocity.y, velocity.x);
    }
    void setAngle(float angle)
    {
        this->angle = angle;
        this->velocity.x = getSpeed() * cos(angle);
        this->velocity.y = getSpeed() * sin(angle);
    }
    bool collidedScreenVertical()
    {
        if (position.y + collisionBox.y / 2 > SCREEN_HEIGHT) return true;
        if (position.y - collisionBox.y / 2 < 0) return true;
        return false;
    }

    bool collidedScreenHorizontal()
    {
        if (position.x + collisionBox.x / 2 > SCREEN_WIDTH) return true;
        if (position.x - collisionBox.x / 2 < 0) return true;
        return false;
    }
    Vector2 calculateWindResistanceForce()
    {
        // F = (1/2) * C * ρ * S * V²，其中F为风阻力，C为风阻系数，ρ为空气密度，S为迎风面积，V为相对速度。这个公式也可以表示为 正面风阻力= 风阻系数× (空气密度x 车头正面投影面积x 车速平方) / 2
        constexpr float AIR_DENSITY = 1.225 * 0.001f;
        float angle = getAngle();
        float windResistanceForce = (1.0f/2.0f) * WIND_FRICTION * AIR_DENSITY * scale.x * scale.y * getSpeedSquare();
        return {windResistanceForce * cos(angle + PI), windResistanceForce * sin(angle + PI)}; // in opposite direction of velocity
    }
    Vector2 calculateGroundFrictionForce()
    {
        float angle = getAngle();
        float groundFrictionForce = GROUND_FRICTION * mass * G;
        if (getSpeedSquare() < 0.01f) return {0, 0};
        return {groundFrictionForce * cos(angle + PI), groundFrictionForce * sin(angle + PI)}; // in opposite direction of velocity
    }
};

class Ball : public GameObject
{
public:
    Ball(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color)
        : GameObject(position, mass, scale, angle, speed, color)
    {
        this->windResistance = false;
        this->groundFriction = false;
    }
    void draw() override
    {
        DrawCircle(this->position.x, this->position.y, this->scale.x, this->color);
    }
    void reset()
    {
        this->position = {SCREEN_WIDTH / 2 + (float)(rand() % 100 - 50), SCREEN_HEIGHT / 2 + (float)(rand() % SCREEN_HEIGHT / 2 - SCREEN_HEIGHT / 4)};
        this->angle = rand() % 120;
        if (this->angle < 60) this->angle = -120 + this->angle;
        this->angle = this->angle - 90;
        this->angle *= PI / 180;
        this->setAngle(this->angle);
        this->setSpeed(BALL_SPEED);
    }
    void handlePaddleCollision(GameObject* paddle)
    {
        if (isCollidingBox(this, paddle)) {
            // Calculate time to roll back to collision point
            float dt = 0.0f;
            
            if (this->velocity.x > 0) {
                float distanceToCollision = (paddle->position.x - paddle->collisionBox.x / 2) - (this->position.x + this->collisionBox.x / 2);
                dt = distanceToCollision / this->velocity.x;
            } else if (this->velocity.x < 0) {
                float distanceToCollision = (this->position.x - this->collisionBox.x / 2) - (paddle->position.x + paddle->collisionBox.x / 2);
                dt = distanceToCollision / this->velocity.x;
            }
            
            // Only roll back if dt is positive and reasonable
            if (dt > 0 && dt < deltaTime) {
                this->position.x -= this->velocity.x * dt;
                this->position.y -= this->velocity.y * dt;
            }

            // check if ball is colliding with paddle because vertical
            if (this->position.y + this->collisionBox.y / 2 > paddle->position.y + paddle->collisionBox.y / 2 && this->position.y - this->collisionBox.y / 2 < paddle->position.y - paddle->collisionBox.y / 2) {
                this->velocity.y = -this->velocity.y; // reflect vertically
            }
            
            this->velocity.x = -this->velocity.x;
        }
    }
    void handleWallCollisions()
    {
        // Handle vertical wall collisions (top/bottom)
        if (collidedScreenVertical()) {
            float dt = 0.0f;
            
            if (this->position.y - this->collisionBox.y / 2 < 0) {
                // Ball hit top wall, calculate time to roll back
                float distanceToCollision = this->collisionBox.y / 2 - this->position.y;
                dt = distanceToCollision / this->velocity.y;
            } else if (this->position.y + this->collisionBox.y / 2 > SCREEN_HEIGHT) {
                // Ball hit bottom wall, calculate time to roll back
                float distanceToCollision = this->position.y + this->collisionBox.y / 2 - SCREEN_HEIGHT;
                dt = distanceToCollision / this->velocity.y;
            }
            
            // Only roll back if dt is positive and reasonable
            if (dt > 0 && dt < deltaTime) {
                this->position.x -= this->velocity.x * dt;
                this->position.y -= this->velocity.y * dt;
            }
            
            this->velocity.y = -this->velocity.y;
        }
    }
    void update(float deltaTime)
    {
        GameObject::update(deltaTime);
        // Handle horizontal walls, when ball is fully outside of screen (left/right) - reset ball
        if (this->position.x + this->collisionBox.x / 2 < 0 || this->position.x - this->collisionBox.x / 2 > SCREEN_WIDTH) {
            if (this->position.x < SCREEN_WIDTH / 2) {
                gRightScore++;
            }
            else {
                gLeftScore++;
            }
            reset();
        }
        
    }
};

class Paddle : public GameObject
{
public:
    Paddle(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color)
        : GameObject(position, mass, scale, angle, speed, color)
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
    void handleVerticalCollision()
    {
        if (collidedScreenVertical()) {
            this->velocity.y = 0;
            this->acceleration.y = 0;
            // Reposition paddle to valid position
            if (this->position.y + this->collisionBox.y / 2 > SCREEN_HEIGHT) {
                this->position.y = SCREEN_HEIGHT - this->collisionBox.y / 2;
            }
            if (this->position.y - this->collisionBox.y / 2 < 0) {
                this->position.y = this->collisionBox.y / 2;
            }
        }
    }
};

class ScoreBoard
{
public:
    Vector2 position;
    Vector2 scale;
    Texture2D* texture;
    ScoreBoard(Vector2 position, Vector2 scale)
        : position(position), scale(scale), texture(nullptr), currentScore(0), drawLeftToRight(false), digitHeight(0.0f)
    {}

    void draw()
    {
        drawScore(currentScore, drawLeftToRight);
    }

    void displayScore(int scoreValue, bool leftToRight = false)
    {
        currentScore = scoreValue;
        drawLeftToRight = leftToRight;
        drawScore(currentScore, drawLeftToRight);
    }

    void setTexture(Texture2D* tex)
    {
        texture = tex;
        if (texture && texture->height > 0)
        {
            digitHeight = static_cast<float>(texture->height) / 10.0f;
        }
        else
        {
            digitHeight = 0.0f;
        }
    }

private:
    int currentScore;
    bool drawLeftToRight;
    float digitHeight;

    void drawScore(int value, bool leftToRight)
    {
        if (!texture || texture->id == 0 || digitHeight <= 0.0f) return;

        if (value < 0) value = 0;
        string digits = to_string(value);

        for (size_t i = 0; i < digits.size(); ++i)
        {
            size_t digitIndex = leftToRight ? i : (digits.size() - 1 - i);
            int digit = digits[digitIndex] - '0';
            digit = max(0, min(9, digit));

            Rectangle source = digitSourceRect(digit);
            Rectangle destination = {
                leftToRight ? position.x + scale.x * static_cast<float>(i)
                            : position.x - scale.x * static_cast<float>(i),
                position.y,
                scale.x,
                scale.y
            };

            DrawTexturePro(*texture, source, destination, {0.0f, 0.0f}, 0.0f, WHITE);
        }
    }

    Rectangle digitSourceRect(int digit) const
    {
        float y = digitHeight * static_cast<float>(digit);
        return {0.0f, y, static_cast<float>(texture->width), digitHeight};
    }
};

ScoreBoard gScoreBoard({SCREEN_WIDTH / 2 - 108.0f, 10.0f}, {72.0f, 72.0f});
ScoreBoard gScoreBoard2({SCREEN_WIDTH / 2 + 36.0f, 10.0f}, {72.0f, 72.0f});
Paddle gPaddle({PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, PADDLE_MASS, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, 0, RED);
Paddle gPaddle2({SCREEN_WIDTH - PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, PADDLE_MASS, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, 0, BLUE);


// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 2: Pong Game");

    gbackground = LoadTexture("assets/pong_background.png");
    gStackedDigitsTexture = LoadTexture("assets/stacked_digits.png");

    // Initialize game objects
    for (int i = 0; i < gBallCount; ++i) {
        Color random_color = {static_cast<unsigned char>(rand() % 256), static_cast<unsigned char>(rand() % 256), static_cast<unsigned char>(rand() % 256), 255};
        Ball gBall({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, BALL_MASS, {BALL_SIZE, BALL_SIZE}, 0, 0, random_color);
        gBall.reset();
        gBalls.push_back(gBall);
    }
    gScoreBoard.setTexture(&gStackedDigitsTexture);
    gScoreBoard2.setTexture(&gStackedDigitsTexture);

    SetTargetFPS(FPS);

    gPaddles.push_back(gPaddle);
    gPaddles.push_back(gPaddle2);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyDown(KEY_ESCAPE)) gAppStatus = TERMINATED;

    // change paddle status if T is pressed
    if (IsKeyDown(KEY_T)) gPaddleStatus = AI_CONTROLLED;

    // handle paddle movement if W, S / UP, DOWN is pressed
    if (IsKeyDown(KEY_W)) gPaddles[0].applyForce({0, -PADDLE_SPEED});
    if (IsKeyDown(KEY_S)) gPaddles[0].applyForce({0, PADDLE_SPEED});
    if (gPaddleStatus == PLAYER_CONTROLLED){
        if (IsKeyDown(KEY_UP)) gPaddles[1].applyForce({0, -PADDLE_SPEED});
        if (IsKeyDown(KEY_DOWN)) gPaddles[1].applyForce({0, PADDLE_SPEED});
    }
    else if (gPaddleStatus == AI_CONTROLLED){
        // should be PID or AI controller
        // TODO: implement PID or AI controller
        // Find the ball closest (in x) to the right paddle
        int nearestBallIdx = 0;
        float minDist = fabs(gBalls[0].position.x - gPaddles[1].position.x);
        for (int i = 1; i < gBalls.size(); ++i) {
            if (gBalls[i].velocity.x < 0) continue; // only consider balls moving towards the right paddle
            float dist = fabs(gBalls[i].position.x - gPaddles[1].position.x);
            if (dist < minDist) {
                minDist = dist;
                nearestBallIdx = i;
            }
        }
        if (gBalls[nearestBallIdx].position.y > gPaddles[1].position.y)
            gPaddles[1].applyForce({0, PADDLE_SPEED});
        else
            gPaddles[1].applyForce({0, -PADDLE_SPEED});
    }


}

void update() {
    updateDeltaTime();

    // handle ball collisions
    for (Ball& ball : gBalls) {
        for (Paddle& paddle : gPaddles) {
            ball.handlePaddleCollision(&paddle);
        }
        ball.handleWallCollisions();
    }
    // handle paddle collisions with screen boundaries
    for (Paddle& paddle : gPaddles) {
        paddle.handleVerticalCollision();
    }

    for (Ball& ball : gBalls) {
        ball.update(deltaTime);
    }
    for (Paddle& paddle : gPaddles) {
        paddle.update(deltaTime);
    }

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
    for (Ball& ball : gBalls) {
        ball.draw();
    }
    for (Paddle& paddle : gPaddles) {
        paddle.draw();
    }
    gScoreBoard.displayScore(gLeftScore, false);
    gScoreBoard2.displayScore(gRightScore, true);

    DrawFPS(10, 10);

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gbackground);
    UnloadTexture(gStackedDigitsTexture);
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

    // Calculate current FPS
    if (deltaTime > 0.0f) {
        gFrameCount++;
        fps = 1.0f / deltaTime;
    } else {
        fps = 0.0f;
    }

    fpsWindow[windowIndex] = fps;
    windowIndex = (windowIndex + 1) % AVG_WINDOW;

    fpsSum = 0.0f;
    for (int i = 0; i < AVG_WINDOW; ++i) {
        fpsSum += fpsWindow[i];
    }
    float movingAvgFps = fpsSum / AVG_WINDOW;

    float minFps = fpsWindow[0];
    for (int i = 1; i < AVG_WINDOW; ++i) {
        if (fpsWindow[i] < minFps) minFps = fpsWindow[i];
    }
    float onePercentLowFps = minFps;

    LOG("FPS: " << fps << ", 1%% Low: " << onePercentLowFps << ", Moving Avg: " << movingAvgFps);
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
    float xDistance = fabs(object1->position.x - object2->position.x) - ((object1->collisionBox.x + object2->collisionBox.x) / 2.0f);
    float yDistance = fabs(object1->position.y - object2->position.y) - ((object1->collisionBox.y + object2->collisionBox.y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}
