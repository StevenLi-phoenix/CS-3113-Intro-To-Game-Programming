#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define MAX_RECTS 1024
#define MAX_TAG_LEN 64

typedef struct {
    Rectangle rect;
    char tag[MAX_TAG_LEN];
} SpriteRect;

int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Atlas Picker - zoom + pan + tag input");

    Texture2D atlas = LoadTexture("ElderAsset.png");
    if (atlas.id == 0) return 1;

    SpriteRect rects[MAX_RECTS];
    int rectCount = 0;

    bool selecting = false;
    Vector2 startPos = {0};
    Vector2 currentPos = {0};

    // 视图控制
    float zoom = 1.0f;
    Vector2 offset = {100, 100};
    bool panning = false;
    Vector2 panStart = {0};

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        // --- 缩放 ---
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            zoom += wheel * 0.1f;
            if (zoom < 0.1f) zoom = 0.1f;
            if (zoom > 8.0f) zoom = 8.0f;
        }

        // --- 平移 ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            panning = true;
            panStart = GetMousePosition();
        }
        if (panning && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))) {
            Vector2 now = GetMousePosition();
            offset.x += (now.x - panStart.x);
            offset.y += (now.y - panStart.y);
            panStart = now;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) || IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
            panning = false;
        }

        // --- 开始左键框选 ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            startPos = GetMousePosition();
            currentPos = startPos;
            selecting = true;
        }

        if (selecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            currentPos = GetMousePosition();
        }

        // --- 松手后创建矩形 + 输入tag ---
        if (selecting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Vector2 endPos = GetMousePosition();

            // 转换为 atlas 坐标（反缩放反偏移）
            float x1 = (startPos.x - offset.x) / zoom;
            float y1 = (startPos.y - offset.y) / zoom;
            float x2 = (endPos.x - offset.x) / zoom;
            float y2 = (endPos.y - offset.y) / zoom;

            Rectangle r = {
                fminf(x1, x2),
                fminf(y1, y2),
                fabsf(x1 - x2),
                fabsf(y1 - y2)
            };

            if (r.width > 2 && r.height > 2 && rectCount < MAX_RECTS) {

                // 输入 tag
                char input[MAX_TAG_LEN] = {0};
                int inputLen = 0;

                while (true) {
                    if (WindowShouldClose()) break;

                    int key = GetKeyPressed();
                    if (key >= 32 && key <= 126 && inputLen < MAX_TAG_LEN - 1) {
                        input[inputLen++] = (char)key;
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && inputLen > 0) {
                        input[--inputLen] = 0;
                    }
                    if (IsKeyPressed(KEY_ENTER) && inputLen > 0) break;

                    BeginDrawing();
                    ClearBackground((Color){30,30,30,255});

                    DrawText("Enter tag name, press ENTER:", 40, 40, 30, RAYWHITE);
                    DrawRectangle(40, 90, 400, 50, DARKGRAY);
                    DrawText(input, 50, 105, 30, WHITE);

                    EndDrawing();
                }

                rects[rectCount].rect = r;
                strncpy(rects[rectCount].tag, input, MAX_TAG_LEN-1);
                rectCount++;
            }

            selecting = false;
        }

        // --- 保存 JSON ---
        if (IsKeyPressed(KEY_S) && rectCount > 0) {
            FILE *f = fopen("atlas.json", "w");
            fprintf(f, "{\n  \"sprites\": [\n");
            for (int i = 0; i < rectCount; i++) {
                Rectangle r = rects[i].rect;
                fprintf(f,
                    "    {\"tag\": \"%s\", \"x\": %d, \"y\": %d, \"w\": %d, \"h\": %d}",
                    rects[i].tag,
                    (int)r.x, (int)r.y, (int)r.width, (int)r.height
                );
                if (i < rectCount - 1) fprintf(f, ",");
                fprintf(f, "\n");
            }
            fprintf(f, "  ]\n}\n");
            fclose(f);
        }

        // --- 绘制 ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTextureEx(atlas, offset, 0, zoom, WHITE);

        // 已有矩形
        for (int i = 0; i < rectCount; i++) {
            Rectangle r = rects[i].rect;
            Rectangle rr = {
                offset.x + r.x * zoom,
                offset.y + r.y * zoom,
                r.width * zoom,
                r.height * zoom
            };
            DrawRectangleLinesEx(rr, 2, RED);
            DrawText(rects[i].tag, rr.x + 4, rr.y + 4, 14, BLACK);
        }

        // 当前框选
        if (selecting) {
            Rectangle temp = {
                fminf(startPos.x, currentPos.x),
                fminf(startPos.y, currentPos.y),
                fabsf(startPos.x - currentPos.x),
                fabsf(startPos.y - currentPos.y)
            };
            DrawRectangleLinesEx(temp, 2, BLUE);
        }

        DrawText("Left drag = select | Right drag = pan | Wheel = zoom | S = save JSON", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
