#ifndef HELPER_H
#define HELPER_H

#define LOG(argument) TraceLog(LOG_DEBUG, argument)
#define LOG_DEBUG(argument) TraceLog(LOG_DEBUG, argument)
#define LOG_INFO(argument) TraceLog(LOG_INFO, argument)
#define LOG_WARNING(argument) TraceLog(LOG_WARNING, argument)
#define LOG_ERROR(argument) TraceLog(LOG_ERROR, argument)
#define LOG_FATAL(argument) TraceLog(LOG_FATAL, argument) // Used to abort program: exit(EXIT_FAILURE)

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
#include <cstdlib>
#include <cstring>

#include "vector_ops.h" // overload for simplify vector operations

enum AppStatus   { TERMINATED, RUNNING };
enum TextureType { SINGLE, ATLAS       };

// logging helper functions
void init_log_level(int argc, char *argv[]);

// Color Helper Functions
Color ColorFromHex(const char *hex);

// Vector Helper Functions
void Normalise(Vector2 *vector);
float GetLength(const Vector2 vector);

// angle helper functions
float normaliseAngle(float angle);
float getAngle(const Vector2 vector);

// Texture Helper Functions
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols);


#endif // HELPER_H