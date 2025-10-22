#include "math3d.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

Color RandomColor() {
    return Color{
        static_cast<unsigned char>(std::rand() % 255),
        static_cast<unsigned char>(std::rand() % 255),
        static_cast<unsigned char>(std::rand() % 255),
        255
    };
}

Cube::Cube(Vector3 position, Vector3 rotation, Vector3 scale, Color color, bool debugFlag)
    : position(position),
      rotation(rotation),
      scale(scale),
      color(color),
      vertices({
          Vector3{0.0f, 0.0f, 0.0f},
          Vector3{0.0f, 0.0f, 1.0f},
          Vector3{0.0f, 1.0f, 0.0f},
          Vector3{0.0f, 1.0f, 1.0f},
          Vector3{1.0f, 0.0f, 0.0f},
          Vector3{1.0f, 0.0f, 1.0f},
          Vector3{1.0f, 1.0f, 0.0f},
          Vector3{1.0f, 1.0f, 1.0f}
      }),
      debug(debugFlag) {
    for (auto& face : faceColors) {
        face = RandomColor();
    }
}

void Cube::SetDebug(bool value) {
    debug = value;
}

bool Cube::ProjectToScreen(const CameraSettings& camera, const Vector3& vertex, Vector2& out) const {
    Vector3 t = vertex * scale - 0.5f * scale;
    t = rotateVec3(t, rotation) + position;
    Vector3 cameraSpace = t - camera.position;
    cameraSpace = q_rotate(cameraSpace, q_conj(camera.orientation));

    if (cameraSpace.z <= camera.nearPlane) {
        return false;
    }

    float focal = 1.0f / std::tan(camera.fovDegrees * PI / 360.0f);
    float aspect = static_cast<float>(camera.screenWidth) / static_cast<float>(camera.screenHeight);
    float x = (cameraSpace.x * focal / cameraSpace.z) / aspect;
    float y = (cameraSpace.y * focal / cameraSpace.z);

    out.x = x * (camera.screenWidth * 0.5f) + (camera.screenWidth * 0.5f);
    out.y = y * (camera.screenHeight * 0.5f) + (camera.screenHeight * 0.5f);
    return true;
}

void Cube::Draw(const CameraSettings& camera) const {
    std::array<Vector2, 8> projected{};
    std::array<bool, 8> visible{};

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        visible[i] = ProjectToScreen(camera, vertices[i], projected[i]);
    }

    auto drawFace = [&](int a, int b, int c, Color faceColor) {
        if (visible[a] && visible[b] && visible[c]) {
            DrawTriangle(projected[a], projected[b], projected[c], faceColor);
        }
    };

    drawFace(0, 1, 2, faceColors[0]);  drawFace(1, 3, 2, faceColors[1]);
    drawFace(4, 6, 5, faceColors[2]);  drawFace(5, 6, 7, faceColors[3]);
    drawFace(0, 2, 4, faceColors[4]);  drawFace(2, 6, 4, faceColors[5]);
    drawFace(1, 5, 3, faceColors[6]);  drawFace(3, 5, 7, faceColors[7]);
    drawFace(2, 3, 6, faceColors[8]);  drawFace(3, 7, 6, faceColors[9]);
    drawFace(0, 4, 1, faceColors[10]); drawFace(1, 4, 5, faceColors[11]);

    if (!debug) {
        return;
    }

    auto drawWireFace = [&](int a, int b, int c, Color wireColor) {
        if (!(visible[a] && visible[b] && visible[c])) {
            return;
        }
        Vector2 pa = projected[a];
        Vector2 pb = projected[b];
        Vector2 pc = projected[c];

        float area = (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
        if (area > 0.0f) {
            std::swap(pb, pc);
        }

        DrawTriangleLines(pa, pb, pc, wireColor);
    };

    drawWireFace(0, 1, 2, BLACK);  drawWireFace(1, 3, 2, BLACK);
    drawWireFace(4, 6, 5, BLACK);  drawWireFace(5, 6, 7, BLACK);
    drawWireFace(0, 2, 4, BLACK);  drawWireFace(2, 6, 4, BLACK);
    drawWireFace(1, 5, 3, BLACK);  drawWireFace(3, 5, 7, BLACK);
    drawWireFace(2, 3, 6, BLACK);  drawWireFace(3, 7, 6, BLACK);
    drawWireFace(0, 4, 1, BLACK);  drawWireFace(1, 4, 5, BLACK);
}

Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vlen(const Vector3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 norm(const Vector3& v) {
    float length = vlen(v);
    if (length > 0.0f) {
        return Vector3{v.x / length, v.y / length, v.z / length};
    }
    return Vector3{0.0f, 0.0f, 0.0f};
}

Vector3 rotateVec3(Vector3 v, Vector3 rot) {
    float cx = std::cos(rot.x);
    float sx = std::sin(rot.x);
    float cy = std::cos(rot.y);
    float sy = std::sin(rot.y);
    float cz = std::cos(rot.z);
    float sz = std::sin(rot.z);

    Vector3 pitch = {
        v.x,
        v.y * cx - v.z * sx,
        v.y * sx + v.z * cx
    };

    Vector3 yaw = {
        pitch.x * cy + pitch.z * sy,
        pitch.y,
        -pitch.x * sy + pitch.z * cy
    };

    Vector3 roll = {
        yaw.x * cz - yaw.y * sz,
        yaw.x * sz + yaw.y * cz,
        yaw.z
    };

    return roll;
}

Vector3 rotateVec3Inv(Vector3 v, Vector3 rot) {
    float cx = std::cos(rot.x);
    float sx = std::sin(rot.x);
    float cy = std::cos(rot.y);
    float sy = std::sin(rot.y);
    float cz = std::cos(rot.z);
    float sz = std::sin(rot.z);

    Vector3 t1 = { v.x,  v.y * cx + v.z * sx, -v.y * sx + v.z * cx };
    Vector3 t2 = { t1.x * cy - t1.z * sy, t1.y, t1.x * sy + t1.z * cy };
    Vector3 t3 = { t2.x * cz + t2.y * sz, -t2.x * sz + t2.y * cz, t2.z };

    return t3;
}

Quaternion q_identity() {
    return Quaternion{0.0f, 0.0f, 0.0f, 1.0f};
}

Quaternion q_normalize(Quaternion q) {
    float length = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (length > 0.0f) {
        return Quaternion{q.x / length, q.y / length, q.z / length, q.w / length};
    }
    return q_identity();
}

Quaternion q_from_axis_angle(Vector3 axis, float angle) {
    Vector3 normalizedAxis = norm(axis);
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    float c = std::cos(halfAngle);
    return Quaternion{normalizedAxis.x * s, normalizedAxis.y * s, normalizedAxis.z * s, c};
}

Quaternion q_mul(Quaternion a, Quaternion b) {
    return Quaternion{
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

Quaternion q_conj(Quaternion q) {
    return Quaternion{-q.x, -q.y, -q.z, q.w};
}

Vector3 q_rotate(Vector3 v, Quaternion q) {
    Quaternion vq{v.x, v.y, v.z, 0.0f};
    Quaternion result = q_mul(q_mul(q, vq), q_conj(q));
    return Vector3{result.x, result.y, result.z};
}
