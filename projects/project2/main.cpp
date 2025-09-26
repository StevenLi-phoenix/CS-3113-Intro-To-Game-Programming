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
 * Time: 2025/09/26 03:19PM
 * Assignment link: TBD
 * File UUID: TBD
 */

 #include "raylib.h"

 // Enums
 enum AppStatus { TERMINATED, RUNNING };
 
 // Global Constants
 constexpr int SCREEN_WIDTH        = 800 * 1.5f,
               SCREEN_HEIGHT       = 450 * 1.5f,
               FPS                 = 60;
 
 // Global Variables
 AppStatus gAppStatus   = RUNNING;
 
 // Function Declarations
 void initialise();
 void processInput();
 void update();
 void render();
 void shutdown();
 
 // Function Definitions
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
 
// // Draw full grey rectangle (fills window)
// DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GRAY);
// // Draw grey circle at center
// DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 100, DARKGRAY);