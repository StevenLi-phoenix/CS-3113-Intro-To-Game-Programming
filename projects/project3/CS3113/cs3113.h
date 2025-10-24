#ifndef CS3113_H
#define CS3113_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

extern bool gDebugMode;
void SetDebugMode(bool enabled);

#define LOG(argument) do { if (gDebugMode) std::cout << argument << '\n'; } while (0)

enum AppStatus   { TERMINATED, RUNNING };
enum TextureType { SINGLE, ATLAS       };

Color ColorFromHex(const char *hex);
void Normalise(Vector2 *vector);
float GetLength(const Vector2 vector);
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols);

#endif // CS3113_H
