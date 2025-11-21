#include "mapgenerator.h"

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace
{
    // Wang hash for fast, decorrelated 32-bit mixing
    inline uint32_t wangHash(uint32_t x)
    {
        x = (x ^ 61u) ^ (x >> 16u);
        x *= 9u;
        x = x ^ (x >> 4u);
        x *= 0x27d4eb2du;
        x = x ^ (x >> 15u);
        return x;
    }
}

MapGenerator::MapGenerator(unsigned int seed)
{
    if (seed == 0)
    {
        std::random_device rd;
        mSeed = rd();
    }
    else
    {
        mSeed = seed;
    }

    buildPermutation();
}

void MapGenerator::buildPermutation()
{
    std::array<int, 256> base;
    std::iota(base.begin(), base.end(), 0);
    std::mt19937 generator(mSeed);
    std::shuffle(base.begin(), base.end(), generator);

    for (size_t i = 0; i < base.size(); ++i)
    {
        mPermutation[i] = base[i];
        mPermutation[256 + i] = base[i];
    }
}

float MapGenerator::fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float MapGenerator::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float MapGenerator::gradPerlin(int hash, float x, float y)
{
    switch (hash & 3)
    {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        default: return 0.0f;
    }
}

float MapGenerator::gradSimplex(int hash, float x, float y)
{
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

float MapGenerator::whiteNoise(int x, int y, unsigned int salt) const
{
    uint32_t xi = static_cast<uint32_t>(static_cast<int32_t>(x));
    uint32_t yi = static_cast<uint32_t>(static_cast<int32_t>(y));
    uint32_t seedMix = wangHash(static_cast<uint32_t>(mSeed) ^ 0x85ebca6bu);
    uint32_t saltMix = wangHash(salt ^ 0xc2b2ae35u);

    uint32_t v = xi * 0x27d4eb2du;
    v ^= yi * 0x165667b1u;
    v ^= seedMix;
    v = wangHash(v ^ saltMix);
    v ^= wangHash(xi + 0x9e3779b9u);
    v ^= wangHash(yi + 0x94d049bbu);
    v = wangHash(v);

    return static_cast<float>(v) / 4294967295.0f;
}

float MapGenerator::perlin(float x, float y) const
{
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    float u = fade(xf);
    float v = fade(yf);

    int aa = mPermutation[mPermutation[xi] + yi];
    int ab = mPermutation[mPermutation[xi] + yi + 1];
    int ba = mPermutation[mPermutation[xi + 1] + yi];
    int bb = mPermutation[mPermutation[xi + 1] + yi + 1];

    float x1 = lerp(gradPerlin(aa, xf, yf), gradPerlin(ba, xf - 1.0f, yf), u);
    float x2 = lerp(gradPerlin(ab, xf, yf - 1.0f), gradPerlin(bb, xf - 1.0f, yf - 1.0f), u);

    return (lerp(x1, x2, v) + 1.0f) * 0.5f;
}

float MapGenerator::simplex(float x, float y) const
{
    constexpr float F2 = 0.366025403784f; // (sqrt(3) - 1) / 2
    constexpr float G2 = 0.211324865405f; // (3 - sqrt(3)) / 6

    float s = (x + y) * F2;
    int i = static_cast<int>(std::floor(x + s));
    int j = static_cast<int>(std::floor(y + s));

    float t = (i + j) * G2;
    float X0 = i - t;
    float Y0 = j - t;
    float x0 = x - X0;
    float y0 = y - Y0;

    int i1 = x0 > y0 ? 1 : 0;
    int j1 = x0 > y0 ? 0 : 1;

    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    int ii = i & 255;
    int jj = j & 255;

    float n0 = 0.0f;
    float n1 = 0.0f;
    float n2 = 0.0f;

    float t0 = 0.5f - x0 * x0 - y0 * y0;
    if (t0 > 0.0f)
    {
        t0 *= t0;
        n0 = t0 * t0 * gradSimplex(mPermutation[ii + mPermutation[jj]], x0, y0);
    }

    float t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 > 0.0f)
    {
        t1 *= t1;
        n1 = t1 * t1 * gradSimplex(mPermutation[ii + i1 + mPermutation[jj + j1]], x1, y1);
    }

    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 > 0.0f)
    {
        t2 *= t2;
        n2 = t2 * t2 * gradSimplex(mPermutation[ii + 1 + mPermutation[jj + 1]], x2, y2);
    }

    float value = 70.0f * (n0 + n1 + n2);
    return (value + 1.0f) * 0.5f;
}

std::vector<unsigned int> MapGenerator::generate(const GenerationSettings &settings, int maxTileIndex) const
{
    if (settings.columns <= 0 || settings.rows <= 0 || maxTileIndex <= 0)
    {
        return {};
    }

    std::vector<unsigned int> levelData(settings.columns * settings.rows, 0);

    for (int row = 0; row < settings.rows; ++row)
    {
        for (int col = 0; col < settings.columns; ++col)
        {
            const int worldX = settings.startX + col;
            const int worldY = settings.startY + row;
            levelData[row * settings.columns + col] = sampleTile(worldX, worldY, settings, maxTileIndex);
        }
    }

    return levelData;
}

unsigned int MapGenerator::sampleTile(int worldX, int worldY, const GenerationSettings &settings, int maxTileIndex) const
{
    if (maxTileIndex <= 0)
    {
        return 0;
    }

    const float zeroBias = std::clamp(settings.zeroBias, 0.0f, 0.999f);
    auto sampleToTile = [&](float value) -> unsigned int
    {
        if (value < zeroBias)
        {
            return 1;
        }

        float remapped = (value - zeroBias) / (1.0f - zeroBias);
        remapped = std::clamp(remapped, 0.0f, 1.0f);

        if (maxTileIndex == 1)
        {
            return 1;
        }

        int tileIndex = static_cast<int>(std::floor(remapped * static_cast<float>(maxTileIndex - 1))) + 2;
        tileIndex = std::clamp(tileIndex, 2, maxTileIndex);

        return static_cast<unsigned int>(tileIndex);
    };

    const float weightSum = std::max(settings.perlinWeight + settings.simplexWeight, 0.0001f);

    if (settings.discreteRandom)
    {
        const int sampleX = worldX + static_cast<int>(settings.offsetX);
        const int sampleY = worldY + static_cast<int>(settings.offsetY);
        float noise = whiteNoise(sampleX, sampleY, settings.discreteSalt);
        return sampleToTile(noise);
    }

    const double scale = std::max(settings.scale, 0.001f);
    const double sampleXBase = (static_cast<double>(worldX) + static_cast<double>(settings.offsetX)) / scale;
    const double sampleYBase = (static_cast<double>(worldY) + static_cast<double>(settings.offsetY)) / scale;

    float amplitude = 1.0f;
    float frequency = 1.0f;
    float amplitudeSum = 0.0f;
    float perlinAccum = 0.0f;
    float simplexAccum = 0.0f;

    for (int octave = 0; octave < settings.octaves; ++octave)
    {
        const float px = static_cast<float>(sampleXBase * frequency);
        const float py = static_cast<float>(sampleYBase * frequency);

        perlinAccum += perlin(px, py) * amplitude;
        simplexAccum += simplex(px + 37.0f, py + 17.0f) * amplitude;

        amplitudeSum += amplitude;
        amplitude *= settings.persistence;
        frequency *= settings.lacunarity;
    }

    if (amplitudeSum > 0.0f)
    {
        perlinAccum /= amplitudeSum;
        simplexAccum /= amplitudeSum;
    }

    float combined = (perlinAccum * settings.perlinWeight + simplexAccum * settings.simplexWeight) / weightSum;
    combined = std::clamp(combined, 0.0f, 1.0f);
    return sampleToTile(combined);
}
