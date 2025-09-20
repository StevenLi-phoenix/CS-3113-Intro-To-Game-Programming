/**
* Author: Steven Li
* Assignment: Simple 2D Scene
* Date due: 2025-09-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
 * Author: Steven Li
 * Time: 2025/09/20 03:12AM
 * Assignment link: https://brightspace.nyu.edu/d2l/lms/dropbox/user/folder_submit_files.d2l?db=1079370&grpid=0&isprv=0&bp=0&ou=501465
 * File UUID: 570e9788-b27e-42a5-8013-a0d2cd853ce3
 */

#include "CS3113/cs3113.h" // Credit to the @sebastianromerocruz/CS-3113-Intro-To-Game-Programming
#include <math.h>
#include <vector>
#include <iostream>
using namespace std;

// Global Constants
constexpr int SCREEN_WIDTH = 1600,
               SCREEN_HEIGHT = 900,
               FPS = 60;
constexpr char BG_COLOUR[] = "#000000";
constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr char TITLE[] = "Project 1: Draw a Simple 2D Scene";
constexpr char TEXTURE_PATH[] = "assets/merged_texture.png";
constexpr Vector2 TEXTURE_BASE_SIZE = { 210, 210 };
// constexpr float G = 6.67430e-11; // gravitational constant
constexpr float G = 100.0f; // gravitational constant
enum PlanetID { EARTH, MOON, SUN };
constexpr Vector2 PLANET_UV_MAP[3] = {
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 0.0f, 1.0f },
};
constexpr float PLANET_MASS_MAP[3] = { 100.0f, 1.0f, 10000.0f };

// OPTIONS
// APPLY SCALE FACTOR TO PLANET BASED ON GRAVITY
constexpr bool APPLY_SCALE_FACTOR_TO_PLANET_BASED_ON_GRAVITY = true;
constexpr float REDUCE_PLANET_SCALE_FACTOR_BY = 50.0f;
// APPLY ROTATION TO PLANET BASED ON GRAVITY ANGLE
constexpr bool APPLY_ROTATION_TO_PLANET_BASED_ON_GRAVITY_ANGLE = true;
// ENABLE FLASHING BACKGROUND
constexpr bool ENABLE_FLASHING_BACKGROUND = true;


// Global Variables
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;
Texture2D gTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void drawTexture(Vector2 uv_top_left, Vector2 texture_size, Vector2 origin, Vector2 px_position, float scale, float angle, Color tint);
Vector2 operator+(const Vector2& a, const Vector2& b);

// User-defined Classes
class GameObject {
    public:
        Vector2 px_position;
        float scale;
        float angle;
        Color tint;

        float scale_factor = 1.0f;

        Vector2 uv_top_left;
        Vector2 texture_size;
        Vector2 origin;

        GameObject(Vector2 uv_top_left) {
            this->uv_top_left = uv_top_left;
            this->texture_size = TEXTURE_BASE_SIZE;
            this->origin = { 0.5f, 0.5f };
            this->px_position = ORIGIN;
            this->scale = 1.0f;
            this->angle = 0.0f;
            this->tint = WHITE;
        }
        void draw() {
            drawTexture(uv_top_left, texture_size, origin, px_position, scale * scale_factor, angle, tint);
        }
};

class planet : public GameObject {
    public:
        float mass;
        Vector2 velocity;
        Vector2 acceleration;
        planet(Vector2 uv_top_left, float mass) : GameObject(uv_top_left) {
            this->mass = mass;
            this->velocity = { 0.0f, 0.0f };
            this->acceleration = { 0.0f, 0.0f };
        }
        void applyForce(Vector2 force) {
            acceleration.x += force.x / mass;
            acceleration.y += force.y / mass;
        }
        void update(float deltaTime) {
            // Update physics
            velocity.x += acceleration.x * deltaTime;
            velocity.y += acceleration.y * deltaTime;
            px_position.x += velocity.x * deltaTime;
            px_position.y += velocity.y * deltaTime;
            acceleration.x = 0.0f; // reset acceleration
            acceleration.y = 0.0f; // reset acceleration
        }
};

vector<planet> gPlanets;
Vector2 calculateGravitationalForce(const planet& p1, const planet& p2);

// Function Definitions
void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    gTexture = LoadTexture(TEXTURE_PATH);

    // Load UV textures to gameObjects
    // enum planet { EARTH, MOON, SUN };
    gPlanets = {
        planet(PLANET_UV_MAP[EARTH], PLANET_MASS_MAP[EARTH]), // earth
        planet(PLANET_UV_MAP[MOON], PLANET_MASS_MAP[MOON]), // moon
        planet(PLANET_UV_MAP[SUN], PLANET_MASS_MAP[SUN]), // sun
    };

    gPlanets[EARTH].px_position = gPlanets[SUN].px_position + Vector2{ 200.0f, 0.0f };
    gPlanets[EARTH].velocity = Vector2{ 0.0f, -75.0f };
    gPlanets[EARTH].scale = 0.2f;
    gPlanets[MOON].px_position = gPlanets[EARTH].px_position + Vector2{ 30.0f, 0.0f };
    gPlanets[MOON].velocity = Vector2{ 0.0f, -87.0f };
    gPlanets[MOON].scale = 0.1f;

    SetTargetFPS(FPS);
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Calculate gravitational force
    for (int i = 0; i < gPlanets.size(); i++) {
        Vector2 gravitationalForce = { 0.0f, 0.0f };
        for (int j = 0; j < gPlanets.size(); j++) {
            if (i != j) {
                Vector2 force = calculateGravitationalForce(gPlanets[i], gPlanets[j]);
                gravitationalForce.x += force.x;
                gravitationalForce.y += force.y;
            }
        }
        if (APPLY_SCALE_FACTOR_TO_PLANET_BASED_ON_GRAVITY) {
            gPlanets[i].scale_factor = sqrt(sqrt(gravitationalForce.x * gravitationalForce.x + gravitationalForce.y * gravitationalForce.y)) / REDUCE_PLANET_SCALE_FACTOR_BY;
        }
        if (APPLY_ROTATION_TO_PLANET_BASED_ON_GRAVITY_ANGLE) {
            gPlanets[i].angle = atan2(gravitationalForce.y, gravitationalForce.x) * 180.0f / PI;
        }

        gPlanets[i].applyForce(gravitationalForce);
    }

    // Update gameObjects physics
    gPlanets[EARTH].update(deltaTime);
    gPlanets[MOON].update(deltaTime);
    gPlanets[SUN].update(deltaTime);

    LOG("EARTH: " << gPlanets[EARTH].px_position.x << ", " << gPlanets[EARTH].px_position.y << endl << "VELOCITY: " << gPlanets[EARTH].velocity.x << ", " << gPlanets[EARTH].velocity.y);
    LOG("MOON: " << gPlanets[MOON].px_position.x << ", " << gPlanets[MOON].px_position.y << endl << "VELOCITY: " << gPlanets[MOON].velocity.x << ", " << gPlanets[MOON].velocity.y);
    LOG("SUN: " << gPlanets[SUN].px_position.x << ", " << gPlanets[SUN].px_position.y << endl << "VELOCITY: " << gPlanets[SUN].velocity.x << ", " << gPlanets[SUN].velocity.y);

}

void render() {
    BeginDrawing();
    
    if (ENABLE_FLASHING_BACKGROUND) {
        // Flash the background randomly
        // Use explicit cast to unsigned char to avoid narrowing errors
        Color flashColor = {
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            static_cast<unsigned char>(rand() % 256),
            255
        };
        ClearBackground(flashColor);
    } else {
        ClearBackground(ColorFromHex(BG_COLOUR));
    }

    // Draw gameObjects (in order of sun -> earth -> moon)
    gPlanets[SUN].draw();
    gPlanets[EARTH].draw();
    gPlanets[MOON].draw();

    EndDrawing();
}

void shutdown() {
    CloseWindow();
}

// User-defined Functions
void drawTexture(Vector2 uv_top_left, Vector2 texture_size, Vector2 origin, Vector2 px_position, float scale, float angle, Color tint) {
    // Vector2 uv_top_left = { 0.0f, 0.0f }; // top left of the texture uv coordinates, normalized to 0-1
    // Vector2 texture_size = { 210, 210 }; // size(width, height) of the texture
    // Vector2 origin = { 0.5f, 0.5f }; // center of the texture
    // Vector2 px_position = { 0.0f, 0.0f }; // where to draw the texture
    // float scale = 1.0f; // scale of the texture
    // float angle = 0.0f; // angle of the texture
    // Color tint = WHITE; // color of the texture
    Rectangle textureArea = { uv_top_left.x * texture_size.x, uv_top_left.y * texture_size.y, texture_size.x, texture_size.y };
    Rectangle destinationArea = { px_position.x, px_position.y, scale * texture_size.x, scale * texture_size.y };
    Vector2 objectOrigin = { origin.x * destinationArea.width, origin.y * destinationArea.height };
    DrawTexturePro(gTexture, textureArea, destinationArea, objectOrigin, angle, tint);
}

Vector2 operator+(const Vector2& a, const Vector2& b) {
    return { a.x + b.x, a.y + b.y };
}


Vector2 calculateGravitationalForce(const planet& p1, const planet& p2) {
    // Classic gravitational force formula F = G * m1 * m2 / r^2
    float dx = p2.px_position.x - p1.px_position.x;
    float dy = p2.px_position.y - p1.px_position.y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance < 1.0f) distance = 1.0f; // prevent divide by zero
    float forceMagnitude = (G * p1.mass * p2.mass) / (distance * distance);
    // Normalize direction
    Vector2 direction = { dx / distance, dy / distance };
    return { forceMagnitude * direction.x, forceMagnitude * direction.y };
}


int main(void) {
    initialise();

    while (gAppStatus == RUNNING) {
        processInput();
        update();
        render();
    }
}