/**
* Author: Steven Li
* Assignment: Pong Clone
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
* Author: Steven Li
* Time: 2025/09/26 18:14PM
* Time: 2025/09/29 12:19PM
* Assignment link: https://brightspace.nyu.edu/d2l/lms/dropbox/user/folder_submit_files.d2l?db=1079373&grpid=0&isprv=0&bp=0&ou=501465
* File UUID: aad8f6da-db4c-4785-9805-12143d63f887
*/

#include "CS3113/cs3113.h"
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

enum PaddleStatus { PLAYER_CONTROLLED, AI_CONTROLLED, NIGHTMARE_CONTROLLED, GOD_CONTROLLED};
enum MouseOperate { MOUSE_DID_NOT_CLICK, MOUSE_CLICKED };
enum GameState {START, IN_GAME, GAME_OVER};

// Global Constants
constexpr int SCREEN_WIDTH = 800 * 1.5f,
              SCREEN_HEIGHT = 450 * 1.5f,
              FPS = 60;

constexpr int PADDLE_WIDTH = 10;
constexpr int PADDLE_HEIGHT = 100;
constexpr int PADDLE_SPEED = 1000;
constexpr int PADDLE_INIT_POS_SCREEN_MARGIN = SCREEN_WIDTH / 25;
constexpr int PADDLE_MASS = 1;
constexpr int BALL_SPEED = 200;
constexpr float BALL_SPEED_RANDOM_OFFSET = 1.2f;
constexpr int BALL_SIZE = 20;
constexpr int BALL_MASS = 1;
constexpr bool ENABLE_WIND_RESISTANCE = true;
constexpr bool ENABLE_GROUND_FRICTION = true;
constexpr float WIND_FRICTION = 0.01f;
constexpr float GROUND_FRICTION = 0.8f;
constexpr float G = 9.8f;
constexpr int BALL_COUNT = 1;
constexpr Color PADDLE_1_COLOR = RED;
constexpr Color PADDLE_2_COLOR = BLUE;
constexpr bool APPLY_RANDOM_FORCE_ON_COLLISION = true;
// ONLY affects AI_CONTROLLED
constexpr float AI_CONTROLLED_PADDLE_SPEED = PADDLE_SPEED;
constexpr float AI_CONTROLLED_IGNORE_DISTANCE = 50;
// TODO: CHECK, set 1 for submission
constexpr int GAME_TARGET_GAME_OVER_SCORE = 1; // 0 means no game over score
// TODO: CHECK, set 3 for submission
constexpr int HARD_BALL_COUNT_CAP = 3;

// Forward declarations
class GameObject;
class Ball;
class Paddle;

// Global Variables
AppStatus gAppStatus = RUNNING;
GameState gGameState = START;
Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
Texture2D gbackground;
Texture2D gStackedDigitsTexture;
Texture2D gPaddleTexture;
Texture2D gBallTexture;
Texture2D gStartScreen;
Texture2D gGameOverScreen;
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
Vector2 gMousePosition = GetMousePosition();
MouseOperate gMouseOperate = MOUSE_DID_NOT_CLICK;


// helper functions
void updateDeltaTime();
bool isCollidingBox(GameObject* object1, GameObject* object2);
Color colorFromInt(int r, int g, int b, int a);
// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void setBallCount(int count);

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
    Texture2D* texture;
    GameObject(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color, Texture2D* texture): position(position), mass(mass), scale(scale), angle(angle), speed(speed), color(color), velocity({0, 0}), acceleration({0, 0}), texture(texture)
    {
        this->velocity.x = speed * cos(angle * PI / 180);
        this->velocity.y = speed * sin(angle * PI / 180);
        this->collisionBox = {scale.x, scale.y};
    }
    virtual ~GameObject() = default;
    void draw()
    {
        DrawTexturePro(
            *texture,
            {0, 0, (float)texture->width, (float)texture->height},
            {this->position.x - this->scale.x / 2, this->position.y - this->scale.y / 2, this->scale.x, this->scale.y},
            {0, 0},
            0,
            color
        );
    }
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
        this->angle = angle * PI / 180;
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
    Ball(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color, Texture2D* texture)
        : GameObject(position, mass, scale, angle, speed, color, texture)
    {
        this->windResistance = false;
        this->groundFriction = false;
    }
    void reset()
    {
        // Color randomness based on time
        // int timenow = (int)(GetTime() * 100);
        // this->color = colorFromInt(timenow % 256, (timenow) % 256, (timenow) % 256, 255);
        
        this->position = {SCREEN_WIDTH / 2 + (float)(rand() % 100 - 50), SCREEN_HEIGHT / 2 + (float)(rand() % SCREEN_HEIGHT / 2 - SCREEN_HEIGHT / 4)};
        this->angle = rand() % 120;
        if (this->angle < 60) this->angle = -120 + this->angle;
        this->angle = this->angle - 90;
        this->setAngle(this->angle);
        float random_offset = (float)(rand() % 100) * BALL_SPEED_RANDOM_OFFSET / 100.0f * BALL_SPEED;
        this->setSpeed(BALL_SPEED + random_offset);
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
            // Cheat a bit to make the ball more consistent
            if (this->position.x > SCREEN_WIDTH / 2) {
                this->velocity.x = -fabs(this->velocity.x);
            } else {
                this->velocity.x = fabs(this->velocity.x);
            }
            if (APPLY_RANDOM_FORCE_ON_COLLISION) {
                if (this->velocity.x > 0) {
                    this->applyForce({getSpeed(),  this->velocity.y ? getSpeed() : -getSpeed()});
                } else {
                    this->applyForce({-getSpeed(), this->velocity.y ? getSpeed() : -getSpeed()});
                }
            }
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
            // check gameover state
            if (this->position.x < SCREEN_WIDTH / 2) {
                gRightScore++;
                if (GAME_TARGET_GAME_OVER_SCORE != 0 && gRightScore >= GAME_TARGET_GAME_OVER_SCORE) {
                    gGameState = GAME_OVER;
                }
            }
            else {
                gLeftScore++;
                if (GAME_TARGET_GAME_OVER_SCORE != 0 && gLeftScore >= GAME_TARGET_GAME_OVER_SCORE) {
                    gGameState = GAME_OVER;
                }
            }
            reset();
        }
        
    }
};

class Paddle : public GameObject
{
public:
    Paddle(Vector2 position, float mass, Vector2 scale, float angle, float speed, Color color, Texture2D* texture)
        : GameObject(position, mass, scale, angle, speed, color, texture)
    {
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
    void reset(){
        this->position = {this->position.x, SCREEN_HEIGHT / 2};
        this->collisionBox = {PADDLE_WIDTH, PADDLE_HEIGHT};
        this->velocity = {0, 0};
        this->acceleration = {0, 0};
        this->angle = 0;
        this->speed = 0;
        this->color = this->color;
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

    void draw(){
        drawScore(currentScore, drawLeftToRight);
    }

    void displayScore(int scoreValue, bool leftToRight = false){
        currentScore = scoreValue;
        drawLeftToRight = leftToRight;
        drawScore(currentScore, drawLeftToRight);
    }

    void setTexture(Texture2D* tex){
        texture = tex;
        if (texture && texture->height > 0){digitHeight = static_cast<float>(texture->height) / 10.0f;}
        else{digitHeight = 0.0f;}
    }

private:
    int currentScore;
    bool drawLeftToRight;
    float digitHeight;

    void drawScore(int value, bool leftToRight){
        if (!texture || texture->id == 0 || digitHeight <= 0.0f) return;

        if (value < 0) value = 0;
        string digits = to_string(value);

        for (size_t i = 0; i < digits.size(); ++i){
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

    Rectangle digitSourceRect(int digit) const {
        float y = digitHeight * static_cast<float>(digit);
        return {0.0f, y, static_cast<float>(texture->width), digitHeight};
    }
};

ScoreBoard gScoreBoard({SCREEN_WIDTH / 2 - 108.0f, 10.0f}, {72.0f, 72.0f});
ScoreBoard gScoreBoard2({SCREEN_WIDTH / 2 + 36.0f, 10.0f}, {72.0f, 72.0f});
Paddle gPaddle({PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, PADDLE_MASS, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, 0, PADDLE_1_COLOR, &gPaddleTexture);
Paddle gPaddle2({SCREEN_WIDTH - PADDLE_INIT_POS_SCREEN_MARGIN, SCREEN_HEIGHT / 2}, PADDLE_MASS, {PADDLE_WIDTH, PADDLE_HEIGHT}, 0, 0, PADDLE_2_COLOR, &gPaddleTexture);

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 2: Pong Game");

    gbackground = LoadTexture("assets/pong_background.png");
    gStackedDigitsTexture = LoadTexture("assets/stacked_digits.png");
    gPaddleTexture = LoadTexture("assets/pong_paddle.png");
    gBallTexture = LoadTexture("assets/pong_ball.png");
    gStartScreen = LoadTexture("assets/start_screen.png");
    gGameOverScreen = LoadTexture("assets/gameover.png");

    // Initialize game objects
    for (int i = 0; i < gBallCount; ++i) { 
        Ball gBall({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, BALL_MASS, {BALL_SIZE, BALL_SIZE}, 0, 0, colorFromInt(rand() % 256, rand() % 256, rand() % 256, 255), &gBallTexture);
        gBall.reset();
        gBalls.push_back(gBall);
    }
    gScoreBoard.setTexture(&gStackedDigitsTexture);
    gScoreBoard2.setTexture(&gStackedDigitsTexture);

    gPaddles.push_back(gPaddle);
    gPaddles.push_back(gPaddle2);
    
    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyDown(KEY_ESCAPE)) gAppStatus = TERMINATED;
    if (gGameState == START){
        if (IsKeyPressed(KEY_ONE)) setBallCount(1);
        if (IsKeyPressed(KEY_TWO)) setBallCount(2);
        if (IsKeyPressed(KEY_THREE)) setBallCount(3);
        if (IsKeyPressed(KEY_ENTER)) {
            for (Paddle& paddle : gPaddles) {
                paddle.reset();
            }
            for (Ball& ball : gBalls) {
                ball.reset();
            }
            gLeftScore = 0;
            gRightScore = 0;
            gPaddleStatus = PLAYER_CONTROLLED;
            gMouseOperate = MOUSE_DID_NOT_CLICK;
            gBallCount = BALL_COUNT;
            gGameState = IN_GAME;
        }
    } else if (gGameState == IN_GAME){
        
        // Reset paddle status if R is pressed
        if (IsKeyDown(KEY_R)) {
            for (Paddle& paddle : gPaddles) {
                paddle.reset();
            }
            for (Ball& ball : gBalls) {
                ball.reset();
            }
            gLeftScore = 0;
            gRightScore = 0;
            gPaddleStatus = PLAYER_CONTROLLED;
            gMouseOperate = MOUSE_DID_NOT_CLICK;
        }

        if (IsKeyDown(KEY_E)) {
            if (gBallCount < HARD_BALL_COUNT_CAP) {
                gBalls.push_back(Ball({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, BALL_MASS, {BALL_SIZE, BALL_SIZE}, 0, 0, colorFromInt(rand() % 256, rand() % 256, rand() % 256, 255), &gBallTexture));
                gBalls.back().reset();
                gBallCount++;
            }
            else {
                LOG("Ball count is at max: " << HARD_BALL_COUNT_CAP);
            }
        }
        if (IsKeyDown(KEY_Q)) {
            if (gBallCount > 1) {
                gBalls.pop_back();
                gBallCount--;
            } // ignore if ball count is 1, collision will break otherwise
        }

        if (IsKeyDown(KEY_ONE) || IsKeyDown(KEY_KP_1)){
            setBallCount(1);
        }
        if (IsKeyDown(KEY_TWO) || IsKeyDown(KEY_KP_2)){
            setBallCount(2);
        }
        if (IsKeyDown(KEY_THREE) || IsKeyDown(KEY_KP_3)){
            setBallCount(3);
        }


        // change paddle status if T is pressed
        if (IsKeyDown(KEY_T)) gPaddleStatus = AI_CONTROLLED;
        if (IsKeyDown(KEY_Y)) gPaddleStatus = NIGHTMARE_CONTROLLED;
        if (IsKeyDown(KEY_U)) gPaddleStatus = GOD_CONTROLLED;

        // handle paddle movement if W, S / UP, DOWN is pressed
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) gMouseOperate = MOUSE_CLICKED;
        if (IsKeyDown(KEY_W)) {
            gMouseOperate = MOUSE_DID_NOT_CLICK;
            gPaddles[0].applyForce({0, -PADDLE_SPEED});
        }
        if (IsKeyDown(KEY_S)) {
            gMouseOperate = MOUSE_DID_NOT_CLICK;
            gPaddles[0].applyForce({0, PADDLE_SPEED});
        }
        if (gMouseOperate == MOUSE_CLICKED){
            gMousePosition = GetMousePosition();
            if (gMousePosition.y > gPaddles[0].position.y) gPaddles[0].applyForce({0, PADDLE_SPEED});
            else if (gMousePosition.y < gPaddles[0].position.y) gPaddles[0].applyForce({0, -PADDLE_SPEED});
        }

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
            float dist = fabs(gBalls[nearestBallIdx].position.y - gPaddles[1].position.y);
            if (gBalls[nearestBallIdx].position.y > gPaddles[1].position.y + AI_CONTROLLED_IGNORE_DISTANCE)
                gPaddles[1].applyForce({0, AI_CONTROLLED_PADDLE_SPEED * (1 - dist / SCREEN_WIDTH)});
            else if (gBalls[nearestBallIdx].position.y < gPaddles[1].position.y - AI_CONTROLLED_IGNORE_DISTANCE)
                gPaddles[1].applyForce({0, -AI_CONTROLLED_PADDLE_SPEED * (1 - dist / SCREEN_WIDTH)});
        } else if (gPaddleStatus == NIGHTMARE_CONTROLLED){
            
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
            gPaddles[1].position.y = gBalls[nearestBallIdx].position.y;
        } else if (gPaddleStatus == GOD_CONTROLLED){
            gPaddles[1].position.y = SCREEN_HEIGHT/2;
            gPaddles[1].collisionBox.x = 40;
            gPaddles[1].collisionBox.y = SCREEN_HEIGHT;
        }
    }
    else if (gGameState == GAME_OVER){
        if (IsKeyPressed(KEY_ENTER)) gGameState = START;
        if (IsKeyPressed(KEY_R)){
            for (Paddle& paddle : gPaddles) {
                paddle.reset();
            }
            for (Ball& ball : gBalls) {
                ball.reset();
            } 
            for (Paddle& paddle : gPaddles) {
                paddle.reset();
            }
            for (Ball& ball : gBalls) {
                ball.reset();
            }
            gLeftScore = 0;
            gRightScore = 0;
            gPaddleStatus = PLAYER_CONTROLLED;
            gMouseOperate = MOUSE_DID_NOT_CLICK;
            gGameState = IN_GAME;
        }
        if (IsKeyPressed(KEY_ESCAPE)) gAppStatus = TERMINATED;
    }
}

void update() {
    updateDeltaTime();

    // check if game is started
    if (gGameState == START){
        return;
    } else if (gGameState == IN_GAME){
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
    else if (gGameState == GAME_OVER){
    }
}

void render()
{
    BeginDrawing();
    // Draw the background
    ClearBackground(GRAY);
    if (gGameState == START){
        // TODO: implement start screen
        DrawTexturePro(
            gStartScreen,
            {0, 0, (float)gStartScreen.width, (float)gStartScreen.height},
            {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT},
            {0, 0},
            0,
            WHITE
        );
        // Draw texture with centered text saying "Press Enter to Start, Press 1, 2, 3 to set ball count, Press T to set ai control mode"
    }
    else if (gGameState == IN_GAME){

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
    }
    else if (gGameState == GAME_OVER){
        Rectangle section;
        if (gLeftScore > gRightScore) {
            // {0,2,1,3}
            section = {0, 2 * (float)gGameOverScreen.height / 4, (float)gGameOverScreen.width, (float)gGameOverScreen.height / 4};
        } else if (gLeftScore < gRightScore) {
            // {0,3,1,4}
            section = {0, 3 * (float)gGameOverScreen.height / 4, (float)gGameOverScreen.width, (float)gGameOverScreen.height / 4};
        } else if (gLeftScore == gRightScore) {
            // Protect case, should never happen
            // {0,0,1,1}
            section = {0, 0 * (float)gGameOverScreen.height / 4, (float)gGameOverScreen.width, (float)gGameOverScreen.height / 4};
        } else {
            // Protect case, should never happen
            // {0,1,1,2}
            section = {0, 1 * (float)gGameOverScreen.height / 4, (float)gGameOverScreen.width, (float)gGameOverScreen.height / 4};
            gScoreBoard.displayScore(gLeftScore, false);
            gScoreBoard2.displayScore(gRightScore, true);
        }
        DrawTexturePro(
            gGameOverScreen,
            section,
            {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT},
            {0, 0},
            0,
            WHITE
        );
       
    }
    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gbackground);
    UnloadTexture(gStackedDigitsTexture);
    UnloadTexture(gPaddleTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gStartScreen);
    UnloadTexture(gGameOverScreen);
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

    if (gFrameCount % 30 == 0) {
        LOG("FPS: " << fps << ", 1%% Low: " << onePercentLowFps << ", Moving Avg: " << movingAvgFps << ", Frame Time: " << deltaTime);
    }
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

Color colorFromInt(int r, int g, int b, int a)
{
    return {static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
}

void setBallCount(int count)
{
    if (count < 1) count = 1;
    if (count > HARD_BALL_COUNT_CAP) count = HARD_BALL_COUNT_CAP;
    // why not clear the balls and add new ones?
    // reseaon: Kept the balls in the same position, so the game is not reset
    gBallCount = count;
    if (gBallCount > gBalls.size()) {
        for (int i = gBalls.size(); i < gBallCount; ++i) {
            gBalls.push_back(Ball({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, BALL_MASS, {BALL_SIZE, BALL_SIZE}, 0, 0, colorFromInt(rand() % 256, rand() % 256, rand() % 256, 255), &gBallTexture));
            gBalls.back().reset();
        }
    }
    else {
        for (int i = gBalls.size(); i > gBallCount; --i) {
            gBalls.pop_back();
        }
    }
}
