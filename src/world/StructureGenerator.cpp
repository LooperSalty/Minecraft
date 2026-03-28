#include "StructureGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include <cstdint>

namespace voxelforge {

static uint32_t hash(int64_t seed, int a, int b, int c) {
    uint64_t h = static_cast<uint64_t>(seed);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(a);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(b);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(c);
    return static_cast<uint32_t>(h >> 16);
}

enum class TreeType { Oak, Birch, Spruce, None };

static TreeType getTreeType(Biome biome) {
    switch (biome) {
        case Biome::Forest:
        case Biome::Plains:
        case Biome::Swamp:
            return TreeType::Oak;
        case Biome::BirchForest:
            return TreeType::Birch;
        case Biome::Taiga:
            return TreeType::Spruce;
        default:
            return TreeType::None;
    }
}

static void placeTree(Chunk& chunk, int bx, int by, int bz,
                      BlockType logType, BlockType leafType, int trunkHeight,
                      uint32_t cornerSeed) {
    // Place trunk
    for (int ty = 1; ty <= trunkHeight; ++ty) {
        int py = by + ty;
        if (py >= CHUNK_HEIGHT) break;
        if (bx >= 0 && bx < CHUNK_WIDTH && bz >= 0 && bz < CHUNK_DEPTH) {
            chunk.setBlock(bx, py, bz, logType);
        }
    }

    // Place leaves: 5x5x3 blob centered at trunk top
    int topY = by + trunkHeight;
    for (int ly = 0; ly < 3; ++ly) {
        int py = topY - 1 + ly;
        if (py < 0 || py >= CHUNK_HEIGHT) continue;

        for (int lx = -2; lx <= 2; ++lx) {
            for (int lz = -2; lz <= 2; ++lz) {
                int px = bx + lx;
                int pz = bz + lz;

                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH) {
                    continue;
                }

                // Remove corners randomly
                bool isCorner = (std::abs(lx) == 2 && std::abs(lz) == 2);
                if (isCorner) {
                    uint32_t ch = hash(cornerSeed, lx + px, ly + py, lz + pz);
                    if ((ch & 1) != 0) {
                        continue;
                    }
                }

                // Don't replace solid blocks (trunk, etc.)
                BlockType existing = chunk.getBlock(px, py, pz);
                if (existing == BlockType::Air || existing == BlockType::OakLeaves ||
                    existing == BlockType::BirchLeaves || existing == BlockType::SpruceLeaves) {
                    chunk.setBlock(px, py, pz, leafType);
                }
            }
        }
    }
}

void StructureGenerator::generateTrees(Chunk& chunk, int64_t seed,
                                       const BiomeGenerator& biomes,
                                       const TerrainGenerator& terrain) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    // Sample biome at chunk center for tree density
    int centerWorldX = chunkX * CHUNK_WIDTH + CHUNK_WIDTH / 2;
    int centerWorldZ = chunkZ * CHUNK_DEPTH + CHUNK_DEPTH / 2;
    Biome centerBiome = biomes.getBiome(centerWorldX, centerWorldZ);
    const BiomeData& biomeData = getBiomeData(centerBiome);

    int maxTrees = biomeData.treeDensity;
    if (maxTrees <= 0) {
        return;
    }

    for (int i = 0; i < maxTrees; ++i) {
        uint32_t h = hash(seed + 9000, chunkX * 100 + i, chunkZ * 100 + i, i);

        int lx = static_cast<int>(h % CHUNK_WIDTH);
        int lz = static_cast<int>((h >> 8) % CHUNK_DEPTH);

        int worldX = chunkX * CHUNK_WIDTH + lx;
        int worldZ = chunkZ * CHUNK_DEPTH + lz;

        Biome localBiome = biomes.getBiome(worldX, worldZ);
        TreeType treeType = getTreeType(localBiome);
        if (treeType == TreeType::None) {
            continue;
        }

        int surfaceY = terrain.getHeight(worldX, worldZ, localBiome);
        if (surfaceY <= 0 || surfaceY >= CHUNK_HEIGHT - 10) {
            continue;
        }

        // Verify surface is suitable
        BlockType surfaceBlock = chunk.getBlock(lx, surfaceY, lz);
        if (surfaceBlock != BlockType::GrassBlock && surfaceBlock != BlockType::Dirt) {
            continue;
        }
        if (surfaceY + 1 >= CHUNK_HEIGHT) {
            continue;
        }
        BlockType aboveBlock = chunk.getBlock(lx, surfaceY + 1, lz);
        if (aboveBlock != BlockType::Air) {
            continue;
        }

        // Determine trunk height and block types
        BlockType logType = BlockType::OakLog;
        BlockType leafType = BlockType::OakLeaves;
        int minHeight = 4;
        int maxHeight = 6;

        switch (treeType) {
            case TreeType::Birch:
                logType = BlockType::BirchLog;
                leafType = BlockType::BirchLeaves;
                minHeight = 5;
                maxHeight = 7;
                break;
            case TreeType::Spruce:
                logType = BlockType::SpruceLog;
                leafType = BlockType::SpruceLeaves;
                minHeight = 4;
                maxHeight = 6;
                break;
            default:
                break;
        }

        int heightRange = maxHeight - minHeight + 1;
        int trunkHeight = minHeight + static_cast<int>((h >> 16) % static_cast<uint32_t>(heightRange));

        placeTree(chunk, lx, surfaceY, lz, logType, leafType, trunkHeight, h);
    }
}

} // namespace voxelforge
