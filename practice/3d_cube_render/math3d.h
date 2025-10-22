#pragma once

#include "raylib.h"
#include "vector_ops.h"

#include <array>

struct CameraSettings {
    Vector3 position;
    Quaternion orientation;
    float fovDegrees;
    float nearPlane;
    float farPlane;
    int screenWidth;
    int screenHeight;
};

Color RandomColor();

class Cube {
public:
    Cube(Vector3 position, Vector3 rotation, Vector3 scale, Color color, bool debug = false);

    void SetDebug(bool value);
    void Draw(const CameraSettings& camera) const;

    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Color color;

private:
    bool ProjectToScreen(const CameraSettings& camera, const Vector3& vertex, Vector2& out) const;

    std::array<Vector3, 8> vertices;
    std::array<Color, 12> faceColors;
    bool debug;
};

// Vector helpers
Vector3 cross(const Vector3& a, const Vector3& b);
float vlen(const Vector3& v);
Vector3 norm(const Vector3& v);
Vector3 rotateVec3(Vector3 v, Vector3 rot);
Vector3 rotateVec3Inv(Vector3 v, Vector3 rot);

// Quaternion helpers
Quaternion q_identity();
Quaternion q_normalize(Quaternion q);
Quaternion q_from_axis_angle(Vector3 axis, float angle);
Quaternion q_mul(Quaternion a, Quaternion b);
Quaternion q_conj(Quaternion q);
Vector3 q_rotate(Vector3 v, Quaternion q);
