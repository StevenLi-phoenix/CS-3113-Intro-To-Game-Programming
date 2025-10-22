#pragma once

#include "raylib.h"

// Component-wise arithmetic between scalar and Vector2
Vector2 operator+(float a, const Vector2& b);
Vector2 operator-(float a, const Vector2& b);
Vector2 operator*(float a, const Vector2& b);
Vector2 operator/(float a, const Vector2& b);
Vector2 operator+(const Vector2& a, float b);
Vector2 operator-(const Vector2& a, float b);
Vector2 operator*(const Vector2& a, float b);
Vector2 operator/(const Vector2& a, float b);

// Component-wise arithmetic between Vector2 values
Vector2 operator+(const Vector2& a, const Vector2& b);
Vector2 operator-(const Vector2& a, const Vector2& b);
Vector2 operator*(const Vector2& a, const Vector2& b);
Vector2 operator/(const Vector2& a, const Vector2& b);

// Component-wise arithmetic between scalar and Vector3
Vector3 operator+(float a, const Vector3& b);
Vector3 operator-(float a, const Vector3& b);
Vector3 operator*(float a, const Vector3& b);
Vector3 operator/(float a, const Vector3& b);
Vector3 operator+(const Vector3& a, float b);
Vector3 operator-(const Vector3& a, float b);
Vector3 operator*(const Vector3& a, float b);
Vector3 operator/(const Vector3& a, float b);

// Component-wise arithmetic between Vector3 values
Vector3 operator+(const Vector3& a, const Vector3& b);
Vector3 operator-(const Vector3& a, const Vector3& b);
Vector3 operator*(const Vector3& a, const Vector3& b);
Vector3 operator/(const Vector3& a, const Vector3& b);
