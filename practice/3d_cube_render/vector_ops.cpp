#include "vector_ops.h"

Vector2 operator+(float a, const Vector2& b) {
    return Vector2{a + b.x, a + b.y};
}

Vector2 operator-(float a, const Vector2& b) {
    return Vector2{a - b.x, a - b.y};
}

Vector2 operator*(float a, const Vector2& b) {
    return Vector2{a * b.x, a * b.y};
}

Vector2 operator/(float a, const Vector2& b) {
    return Vector2{a / b.x, a / b.y};
}

Vector2 operator+(const Vector2& a, float b) {
    return Vector2{a.x + b, a.y + b};
}

Vector2 operator-(const Vector2& a, float b) {
    return Vector2{a.x - b, a.y - b};
}

Vector2 operator*(const Vector2& a, float b) {
    return Vector2{a.x * b, a.y * b};
}

Vector2 operator/(const Vector2& a, float b) {
    return Vector2{a.x / b, a.y / b};
}

Vector2 operator+(const Vector2& a, const Vector2& b) {
    return Vector2{a.x + b.x, a.y + b.y};
}

Vector2 operator-(const Vector2& a, const Vector2& b) {
    return Vector2{a.x - b.x, a.y - b.y};
}

Vector2 operator*(const Vector2& a, const Vector2& b) {
    return Vector2{a.x * b.x, a.y * b.y};
}

Vector2 operator/(const Vector2& a, const Vector2& b) {
    return Vector2{a.x / b.x, a.y / b.y};
}

Vector3 operator+(float a, const Vector3& b) {
    return Vector3{a + b.x, a + b.y, a + b.z};
}

Vector3 operator-(float a, const Vector3& b) {
    return Vector3{a - b.x, a - b.y, a - b.z};
}

Vector3 operator*(float a, const Vector3& b) {
    return Vector3{a * b.x, a * b.y, a * b.z};
}

Vector3 operator/(float a, const Vector3& b) {
    return Vector3{a / b.x, a / b.y, a / b.z};
}

Vector3 operator+(const Vector3& a, float b) {
    return Vector3{a.x + b, a.y + b, a.z + b};
}

Vector3 operator-(const Vector3& a, float b) {
    return Vector3{a.x - b, a.y - b, a.z - b};
}

Vector3 operator*(const Vector3& a, float b) {
    return Vector3{a.x * b, a.y * b, a.z * b};
}

Vector3 operator/(const Vector3& a, float b) {
    return Vector3{a.x / b, a.y / b, a.z / b};
}

Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3{a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 operator*(const Vector3& a, const Vector3& b) {
    return Vector3{a.x * b.x, a.y * b.y, a.z * b.z};
}

Vector3 operator/(const Vector3& a, const Vector3& b) {
    return Vector3{a.x / b.x, a.y / b.y, a.z / b.z};
}
