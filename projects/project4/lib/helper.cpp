#include "helper.h"

#include <algorithm>

Color ColorFromHex(const char *hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    // Default alpha = 255 (opaque)
    unsigned int r = 0, 
                 g = 0, 
                 b = 0, 
                 a = 255;

    // 6‑digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // 8‑digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
        return (Color){ (unsigned char) r,
                        (unsigned char) g,
                        (unsigned char) b,
                        (unsigned char) a };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}

/**
 * @brief Calculates and returns the magnitude of a 2D vector.
 * 
 * @param vector Any 2D raylib vector.
 */
float GetLength(const Vector2 vector)
{
    return sqrtf(
        pow(vector.x, 2) + pow(vector.y, 2)
    );
}

/**
 * @brief Mutates two dimensional vector to become its unit vector counterpart,
 * also known as a direction vector, retains the original vector’s orientation
 * but has a standardised length.
 * 
 * @see https://hogonext.com/how-to-normalize-a-vector/
 * 
 * @param vector Any 2D raylib vector.
 */
void Normalise(Vector2 *vector)
{
    float magnitude = GetLength(*vector);

    vector->x /= magnitude;
    vector->y /= magnitude;
}

/**
 * @brief Calculates and returns the UV coordinates and dimensions of a 
 * rectangle slice from a texture based on the given index, number of rows, and
 * number of columns.
 * 
 * @param texture a pointer to a `Texture2D` struct, contains information about
 * a 2D texture such as its width and height.
 * @param index represents the index of the specific slice within a texture 
 * atlas. Each slice is a sub-image within the texture atlas that contains 
 * multiple images arranged in rows and columns.
 * @param rows represents the number of rows in which the texture is divided.
 * This parameter is used to calculate the vertical position of the texture 
 * slice based on the index provided.
 * @param cols represents the number of columns in a grid layout. It is used in
 * the `getUVRectangle` function to calculate the UV coordinates for a specific
 * index within the grid.
 * 
 * @return a `Rectangle` struct that represents a portion of a texture based on
 * the provided parameters. The `Rectangle` struct contains the top-left 
 * x-coordinate, top-left y-coordinate, width, and height of the specified
 * portion of the texture.
 */
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols)
{
    float uCoord = (float) (index % cols) / (float) cols;
    uCoord *= texture->width;

    float vCoord = (float) (index / cols) / (float) rows;
    vCoord *= texture->height;

    float sliceWidth  = texture->width  / (float) cols;
    float sliceHeight = texture->height / (float) rows;

    return { 
        uCoord,     // top-left x-coord
        vCoord,     // top-left y-coord
        sliceWidth, // width of slice
        sliceHeight // height of slice
    };
}

float normaliseAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

float getAngle(const Vector2 vector)
{
    return normaliseAngle(atan2f(vector.y, vector.x));
}

void init_log_level(int argc, char *argv[])
{
    // 0 = info, 1 = debug
    int debug = 0;

    // compile-time macro (optional fallback)
#ifdef DEBUG
#if DEBUG == 1
debug = 1;
#endif
#endif

    // environment variable (overrides compile-time)
    if (!debug) {
        const char* e = std::getenv("DEBUG");
        if (e && (std::strcmp(e, "1") == 0 || std::strcmp(e, "true") == 0)) {
            debug = 1;
        }
    }

    // command-line flag (highest precedence)
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--debug") == 0) debug = 1;
    }

    // apply log level
    if (debug) {
        SetTraceLogLevel(LOG_DEBUG);
        LOG("Log level: DEBUG");
    } else {
        SetTraceLogLevel(LOG_INFO);
        LOG("Log level: INFO");
    }

}

namespace
{
    int RoundToInt(float value)
    {
        return (value >= 0.0f)
            ? static_cast<int>(value + 0.5f)
            : static_cast<int>(value - 0.5f);
    }
}

void DrawFilledRectangle(const Rectangle &rect, Color color)
{
    const int x = RoundToInt(rect.x);
    const int y = RoundToInt(rect.y);
    const int width = RoundToInt(rect.width);
    const int height = RoundToInt(rect.height);

    DrawRectangle(x, y, width, height, color);
}

void DrawRectangleBorder(const Rectangle &rect, float thickness, Color color)
{
    const int border = std::max(1, RoundToInt(thickness));
    const int x = RoundToInt(rect.x);
    const int y = RoundToInt(rect.y);
    const int width = RoundToInt(rect.width);
    const int height = RoundToInt(rect.height);

    DrawRectangle(x, y, width, border, color);
    DrawRectangle(x, y + height - border, width, border, color);
    DrawRectangle(x, y, border, height, color);
    DrawRectangle(x + width - border, y, border, height, color);
}

Color ApplyAlpha(Color color, float alpha)
{
    float clamped = alpha;
    if (clamped < 0.0f) clamped = 0.0f;
    else if (clamped > 1.0f) clamped = 1.0f;
    const float scaledAlpha = static_cast<float>(color.a) * clamped;
    color.a = static_cast<unsigned char>(roundf(scaledAlpha));
    return color;
}

bool PointInRectangle(Vector2 point, const Rectangle &rect)
{
    const float right = rect.x + rect.width;
    const float bottom = rect.y + rect.height;

    return point.x >= rect.x && point.x <= right &&
           point.y >= rect.y && point.y <= bottom;
}
