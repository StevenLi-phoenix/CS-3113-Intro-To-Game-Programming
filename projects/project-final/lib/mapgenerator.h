#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include <array>
#include <vector>

class MapGenerator
{
public:
    enum class NoiseType { Perlin, Simplex };

    struct GenerationSettings
    {
        int columns = 0;
        int rows = 0;
        float scale = 24.0f;
        int octaves = 4;
        float persistence = 0.5f;
        float lacunarity = 2.0f;
        float perlinWeight = 0.5f;
        float simplexWeight = 0.5f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        int startX = 0; // world-space column to start sampling from
        int startY = 0; // world-space row to start sampling from
        float zeroBias = 0.0f; // 0-1 range, values below bias become tile index 0
        bool discreteRandom = false;
        unsigned int discreteSalt = 0;
    };

    explicit MapGenerator(unsigned int seed = 0);

    std::vector<unsigned int> generate(const GenerationSettings &settings, int maxTileIndex) const;
    unsigned int sampleTile(int worldX, int worldY, const GenerationSettings &settings, int maxTileIndex) const;
    float perlin(float x, float y) const;
    float simplex(float x, float y) const;
    float whiteNoise(int x, int y, unsigned int salt = 0) const;

private:
    std::array<int, 512> mPermutation;
    unsigned int mSeed;

    void buildPermutation();
    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float gradPerlin(int hash, float x, float y);
    static float gradSimplex(int hash, float x, float y);
};

#endif
