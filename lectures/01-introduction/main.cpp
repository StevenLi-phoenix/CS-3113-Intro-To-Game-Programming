#include "raylib.h"

// 枚举
enum AppStatus { TERMINATED, RUNNING };

// 全局常量
constexpr int SCREEN_WIDTH        = 800 * 1.5f,
              SCREEN_HEIGHT       = 450 * 1.5f,
              FPS                 = 60;

// 全局变量
AppStatus gAppStatus   = RUNNING;

// 函数声明
void initialise();
void processInput();
void update();
void render();
void shutdown();

// 函数定义
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello raylib!");

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {}

void render()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow(); // 关闭窗口和OpenGL上下文
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