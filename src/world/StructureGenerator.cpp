#include "StructureGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include "../util/Noise.h"
#include <cstdint>
#include <cmath>

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

void StructureGenerator::generateVegetation(Chunk& chunk, int64_t seed,
                                             const BiomeGenerator& biomes,
                                             const TerrainGenerator& terrain) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();
    int baseSeed = static_cast<int>(seed & 0x7FFFFFFF);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkX * CHUNK_WIDTH + x;
            int worldZ = chunkZ * CHUNK_DEPTH + z;

            Biome biome = biomes.getBiome(worldX, worldZ);
            int surfaceY = terrain.getHeight(worldX, worldZ, biome);

            // Skip if below or at sea level, or too high
            if (surfaceY <= SEA_LEVEL || surfaceY >= CHUNK_HEIGHT - 2) {
                continue;
            }

            // Check that the surface block is suitable
            BlockType surface = chunk.getBlock(x, surfaceY, z);
            BlockType above = chunk.getBlock(x, surfaceY + 1, z);
            if (above != BlockType::Air) {
                continue;
            }

            // Use noise-based distribution for natural clusters
            float fx = static_cast<float>(worldX);
            float fz = static_cast<float>(worldZ);

            // Vegetation density noise: clustered placement
            float vegNoise = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 32.0f, baseSeed + 10000);
            // Flower noise: separate layer for flower clusters
            float flowerNoise = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 48.0f, baseSeed + 11000);

            // Hash for per-block randomness
            uint32_t h = hash(seed + 12000, worldX, worldZ, surfaceY);
            float rng = static_cast<float>(h % 1000) / 1000.0f;

            switch (biome) {
                case Biome::Plains: {
                    if (surface != BlockType::GrassBlock) break;
                    // 30% tall grass where noise is favorable
                    if (vegNoise > 0.0f && rng < 0.30f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    }
                    // 5% flowers in flower-noise clusters
                    else if (flowerNoise > 0.3f && rng < 0.05f) {
                        // Alternate between poppy and dandelion
                        BlockType flower = ((h >> 10) & 1) ? BlockType::Poppy : BlockType::Dandelion;
                        chunk.setBlock(x, surfaceY + 1, z, flower);
                    }
                    break;
                }
                case Biome::Forest:
                case Biome::BirchForest: {
                    if (surface != BlockType::GrassBlock) break;
                    // 15% tall grass
                    if (vegNoise > 0.1f && rng < 0.15f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    }
                    // 2% flowers
                    else if (flowerNoise > 0.4f && rng < 0.02f) {
                        BlockType flower = ((h >> 10) & 1) ? BlockType::Poppy : BlockType::Dandelion;
                        chunk.setBlock(x, surfaceY + 1, z, flower);
                    }
                    break;
                }
                case Biome::Desert: {
                    if (surface != BlockType::Sand) break;
                    // 3% cactus with spacing (use noise to avoid adjacent cacti)
                    if (vegNoise > 0.4f && rng < 0.03f) {
                        // Place 1-3 block tall cactus
                        int cactusH = 1 + static_cast<int>((h >> 4) % 3);
                        for (int cy = 1; cy <= cactusH; ++cy) {
                            if (surfaceY + cy < CHUNK_HEIGHT) {
                                chunk.setBlock(x, surfaceY + cy, z, BlockType::Cactus);
                            }
                        }
                    }
                    break;
                }
                case Biome::Taiga:
                case Biome::ExtremeHills: {
                    // Snow biome: place snow on top if grass, no vegetation
                    if (surface == BlockType::GrassBlock || surface == BlockType::Dirt) {
                        chunk.setBlock(x, surfaceY, z, BlockType::SnowBlock);
                    }
                    break;
                }
                case Biome::Swamp: {
                    if (surface != BlockType::GrassBlock) break;
                    // Swamp has some grass and clay patches near water
                    if (vegNoise > -0.1f && rng < 0.20f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    }
                    // Clay patches near sea level
                    if (surfaceY <= SEA_LEVEL + 2 && flowerNoise < -0.3f) {
                        chunk.setBlock(x, surfaceY, z, BlockType::Clay);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

} // namespace voxelforge
