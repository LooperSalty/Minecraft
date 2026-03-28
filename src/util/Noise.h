#pragma once

namespace voxelforge {

float perlin2D(float x, float z, int seed);
float perlin3D(float x, float y, float z, int seed);
float octavePerlin2D(float x, float z, int octaves, float persistence, float scale, int seed);
float octavePerlin3D(float x, float y, float z, int octaves, float persistence, float scale, int seed);

} // namespace voxelforge
