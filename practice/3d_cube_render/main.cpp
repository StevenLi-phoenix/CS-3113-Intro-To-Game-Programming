#include "raylib.h"
#include "math3d.h"
#include <stdexcept>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH        = 800 * 1.5f,
              SCREEN_HEIGHT       = 450 * 1.5f,
              FPS                 = 60;

// Global Variables
AppStatus gAppStatus   = RUNNING;
Vector3 gCameraPosition = {0, 0, 0};
Vector3 gCameraRotation = {0, 0, 0};
Vector3 gCameraScale = {1, 1, 1};
float gCameraFOV = 60.0f;
float gCameraNearPlane = 0.1f;
float gCameraFarPlane = 100.0f;
float gPreviousTicks = 0.0f;
bool gDebug = false;
Quaternion gCamQ = Quaternion{0,0,0,1};
bool needsResort = true;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void drawCubesByDistance();

vector<Cube> gCubes;

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Triangle");
    SetTargetFPS(FPS);

    for (int x = -10; x < 10; ++x) {
        for (int y = -10; y < 10; ++y) {
            gCubes.emplace_back(
                Vector3{static_cast<float>(x), static_cast<float>(y), 5.0f},
                Vector3{0.0f, 0.0f, 0.0f},
                Vector3{1.0f, 1.0f, 1.0f},
                ORANGE,
                gDebug
            );
        }
    }
}

void processInput() 
{
    if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE) || (IsKeyDown(KEY_LEFT_SUPER) && IsKeyPressed(KEY_W))) gAppStatus = TERMINATED;
    float dt = GetFrameTime();
    float rotSpeed = 1.5f;              // rad/s
    float d = rotSpeed * dt;

    // 本地轴旋转：右轴(X)、上轴(Y)、前轴(Z)
    if (IsKeyDown(KEY_W)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({1,0,0}, +d)) ); // pitch+
    if (IsKeyDown(KEY_S)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({1,0,0}, -d)) ); // pitch-
    if (IsKeyDown(KEY_A)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({0,1,0}, -d)) ); // yaw+
    if (IsKeyDown(KEY_D)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({0,1,0}, +d)) ); // yaw-
    if (IsKeyDown(KEY_Q)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({0,0,1}, -d)) ); // roll+
    if (IsKeyDown(KEY_E)) gCamQ = q_normalize( q_mul(gCamQ, q_from_axis_angle({0,0,1}, +d)) ); // roll-

    // 基向量（包含roll）
    Vector3 forward = norm( q_rotate({0,0,1}, gCamQ) );
    Vector3 right   = norm( q_rotate({1,0,0}, gCamQ) );
    Vector3 up      = norm( q_rotate({0,1,0}, gCamQ) );

    // 连续移动（改用 IsKeyDown）
    float speed = 3.0f;                 // 单位/秒
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed *= 3.0f;

    // 检查是否有移动，标记需要重新排序
    bool moved = false;
    if (IsKeyDown(KEY_J)) { gCameraPosition = gCameraPosition - (speed*dt) * right; moved = true; }
    if (IsKeyDown(KEY_L)) { gCameraPosition = gCameraPosition + (speed*dt) * right; moved = true; }
    if (IsKeyDown(KEY_K)) { gCameraPosition = gCameraPosition + (speed*dt) * up; moved = true; }
    if (IsKeyDown(KEY_I)) { gCameraPosition = gCameraPosition - (speed*dt) * up; moved = true; }
    if (IsKeyDown(KEY_N)) { gCameraPosition = gCameraPosition - (speed*dt) * forward; moved = true; }
    if (IsKeyDown(KEY_H)) { gCameraPosition = gCameraPosition + (speed*dt) * forward; moved = true; }
    
    // 如果有移动，标记需要重新排序
    if (moved) needsResort = true;
}

void update() {
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // translate the camera
    // gCameraPosition.x = sin(ticks);
    // gCameraPosition.y = cos(ticks);
    
    // Rotate the cube
    // gCube.rotation.x = ticks * 0.5f;
    // gCube.rotation.y = ticks * 0.3f;
    // gCube.rotation.z = ticks * 0.7f;
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(10, 10);
    drawCubesByDistance();
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

void drawP(Vector3* vertices, Color color) {
    // divide any vertices into number of triangles
    throw std::runtime_error("At least 3 vertices are required");
}
// 性能优化：缓存距离计算，减少重复排序
static vector<pair<float, int>> cachedDistances;
static Vector3 lastCameraPos = {0, 0, 0};

void drawCubesByDistance() {
    // 检查相机位置是否改变，决定是否需要重新排序
    Vector3 cameraDelta = gCameraPosition - lastCameraPos;
    if (vlen(cameraDelta) > 0.1f || needsResort) {
        lastCameraPos = gCameraPosition;
        needsResort = false;
        
        // 预分配空间，避免重复分配
        cachedDistances.resize(gCubes.size());
        
        // 计算距离（使用平方距离避免开方运算）
        for (int i = 0; i < gCubes.size(); i++) {
            Vector3 delta = gCubes[i].position - gCameraPosition;
            float distSquared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            cachedDistances[i] = {distSquared, i};
        }
        
        // 按距离排序（远到近）
        sort(cachedDistances.begin(), cachedDistances.end(), 
             [](const pair<float, int>& a, const pair<float, int>& b) {
                 return a.first > b.first;
             });
    }
    
    const CameraSettings camera{
        gCameraPosition,
        gCamQ,
        gCameraFOV,
        gCameraNearPlane,
        gCameraFarPlane,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    };

    // 按排序顺序绘制
    for (const auto& entry : cachedDistances) {
        Cube& cube = gCubes[entry.second];
        cube.SetDebug(gDebug);
        cube.Draw(camera);
    }
}
