#pragma once
#include <cstdint>

namespace voxelforge {

class Chunk;

class OreGenerator {
public:
    static void generate(Chunk& chunk, int64_t seed);
    static void generate(Chunk& chunk, int64_t seed, bool isExtremeHills);
};

} // namespace voxelforge
