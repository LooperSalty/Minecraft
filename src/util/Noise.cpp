#include "Noise.h"
#include <cmath>
#include <array>

namespace voxelforge {

// Fade function: 6t^5 - 15t^4 + 10t^3
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Hash-based permutation lookup with seed offset
static int perm(int i, int seed) {
    // Use a simple hash to create a seed-dependent permutation
    constexpr int TABLE_SIZE = 256;
    int v = ((i + seed) * 2654435761) & 0x7FFFFFFF;
    return (v ^ (v >> 16)) & (TABLE_SIZE - 1);
}

static float grad2D(int hash, float x, float z) {
    int h = hash & 3;
    switch (h) {
        case 0: return  x + z;
        case 1: return -x + z;
        case 2: return  x - z;
        default: return -x - z;
    }
}

static float grad3D(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float perlin2D(float x, float z, int seed) {
    int xi = static_cast<int>(std::floor(x)) & 255;
    int zi = static_cast<int>(std::floor(z)) & 255;
    float xf = x - std::floor(x);
    float zf = z - std::floor(z);

    float u = fade(xf);
    float v = fade(zf);

    int aa = perm(perm(xi, seed) + zi, seed);
    int ab = perm(perm(xi, seed) + zi + 1, seed);
    int ba = perm(perm(xi + 1, seed) + zi, seed);
    int bb = perm(perm(xi + 1, seed) + zi + 1, seed);

    float x1 = lerp(grad2D(aa, xf, zf), grad2D(ba, xf - 1.0f, zf), u);
    float x2 = lerp(grad2D(ab, xf, zf - 1.0f), grad2D(bb, xf - 1.0f, zf - 1.0f), u);

    return lerp(x1, x2, v);
}

float perlin3D(float x, float y, float z, int seed) {
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    int zi = static_cast<int>(std::floor(z)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float zf = z - std::floor(z);

    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);

    int aaa = perm(perm(perm(xi, seed) + yi, seed) + zi, seed);
    int aab = perm(perm(perm(xi, seed) + yi, seed) + zi + 1, seed);
    int aba = perm(perm(perm(xi, seed) + yi + 1, seed) + zi, seed);
    int abb = perm(perm(perm(xi, seed) + yi + 1, seed) + zi + 1, seed);
    int baa = perm(perm(perm(xi + 1, seed) + yi, seed) + zi, seed);
    int bab = perm(perm(perm(xi + 1, seed) + yi, seed) + zi + 1, seed);
    int bba = perm(perm(perm(xi + 1, seed) + yi + 1, seed) + zi, seed);
    int bbb = perm(perm(perm(xi + 1, seed) + yi + 1, seed) + zi + 1, seed);

    float x1 = lerp(grad3D(aaa, xf, yf, zf),
                     grad3D(baa, xf - 1.0f, yf, zf), u);
    float x2 = lerp(grad3D(aba, xf, yf - 1.0f, zf),
                     grad3D(bba, xf - 1.0f, yf - 1.0f, zf), u);
    float y1 = lerp(x1, x2, v);

    float x3 = lerp(grad3D(aab, xf, yf, zf - 1.0f),
                     grad3D(bab, xf - 1.0f, yf, zf - 1.0f), u);
    float x4 = lerp(grad3D(abb, xf, yf - 1.0f, zf - 1.0f),
                     grad3D(bbb, xf - 1.0f, yf - 1.0f, zf - 1.0f), u);
    float y2 = lerp(x3, x4, v);

    return lerp(y1, y2, w);
}

float octavePerlin2D(float x, float z, int octaves, float persistence, float scale, int seed) {
    float total = 0.0f;
    float frequency = scale;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlin2D(x * frequency, z * frequency, seed + i) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

float octavePerlin3D(float x, float y, float z, int octaves, float persistence, float scale, int seed) {
    float total = 0.0f;
    float frequency = scale;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlin3D(x * frequency, y * frequency, z * frequency, seed + i) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

} // namespace voxelforge
