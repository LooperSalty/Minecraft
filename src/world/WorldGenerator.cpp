#include "WorldGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include "OreGenerator.h"
#include "StructureGenerator.h"

namespace voxelforge {

WorldGenerator::WorldGenerator(int64_t seed)
    : m_seed(seed)
    , m_biomes(seed)
    , m_terrain(seed) {}

void WorldGenerator::generate(Chunk& chunk) const {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    // Track if this chunk has ExtremeHills for emerald ore
    bool hasExtremeHills = false;

    // Pass 1: fill terrain columns
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkX * CHUNK_WIDTH + x;
            int worldZ = chunkZ * CHUNK_DEPTH + z;

            Biome biome = m_biomes.getBiome(worldX, worldZ);
            int height = m_terrain.getBlendedHeight(worldX, worldZ, m_biomes);

            if (biome == Biome::ExtremeHills) hasExtremeHills = true;

            bool isDesert = (biome == Biome::Desert);
            bool isBeach = (biome == Biome::Beach);

            // Bedrock layer: Y 0-4
            for (int y = 0; y <= 4; ++y) {
                chunk.setBlock(x, y, z, BlockType::Bedrock);
            }

            // Stone layer: Y 5 to (height - 4)
            int stoneTop = height - 4;
            for (int y = 5; y <= stoneTop; ++y) {
                chunk.setBlock(x, y, z, BlockType::Stone);
            }

            // Desert sandstone underlay: Y (height-7) to (height-4)
            if (isDesert) {
                int sandstoneBottom = height - 7;
                int sandstoneTop = height - 4;
                for (int y = sandstoneBottom; y <= sandstoneTop; ++y) {
                    if (y >= 5) {
                        chunk.setBlock(x, y, z, BlockType::Sandstone);
                    }
                }
            }

            // Filler layer: Y (height-3) to (height-1)
            BlockType fillerBlock = BlockType::Dirt;
            if (isDesert || isBeach) {
                fillerBlock = BlockType::Sand;
            } else if (biome == Biome::Ocean) {
                // Ocean floor: mix of dirt, gravel, clay
                fillerBlock = BlockType::Dirt;
            }
            for (int y = height - 3; y <= height - 1; ++y) {
                if (y >= 5) {
                    chunk.setBlock(x, y, z, fillerBlock);
                }
            }

            // Surface block: Y = height
            BlockType surfaceBlock = BlockType::GrassBlock;
            if (isDesert || isBeach) {
                surfaceBlock = BlockType::Sand;
            } else if (biome == Biome::ExtremeHills && height > 100) {
                // Exposed stone at mountain peaks
                surfaceBlock = BlockType::Stone;
            } else if (biome == Biome::ExtremeHills && height > 90) {
                surfaceBlock = BlockType::SnowBlock;
            } else if (biome == Biome::Taiga) {
                surfaceBlock = BlockType::GrassBlock; // Taiga has grass, snow layer added later
            } else if (biome == Biome::MushroomIsland) {
                surfaceBlock = BlockType::Mycelium;
            } else if (biome == Biome::Ocean && height < SEA_LEVEL) {
                surfaceBlock = BlockType::Dirt; // Ocean floor
            }

            if (height >= 1) {
                chunk.setBlock(x, height, z, surfaceBlock);
            }

            // Beach detection: sand strip at water level transitions
            // If the height is within 2 blocks of sea level and not ocean/desert
            if (!isDesert && !isBeach && biome != Biome::Ocean &&
                biome != Biome::MushroomIsland) {
                if (height >= SEA_LEVEL - 1 && height <= SEA_LEVEL + 2) {
                    // Check if there's ocean nearby by looking at continental value
                    float continental = m_terrain.getContinentalValue(worldX, worldZ);
                    if (continental < 0.0f) {
                        // Near ocean: make it sandy (beach transition)
                        chunk.setBlock(x, height, z, BlockType::Sand);
                        for (int y = height - 3; y <= height - 1; ++y) {
                            if (y >= 5) {
                                chunk.setBlock(x, y, z, BlockType::Sand);
                            }
                        }
                    }
                }
            }

            // Water fill: if surface is below sea level
            if (height < SEA_LEVEL) {
                for (int y = height + 1; y <= SEA_LEVEL; ++y) {
                    chunk.setBlock(x, y, z, BlockType::Water);
                }
            }

            // River bed: place gravel/clay at river bottoms
            float riverDist = m_terrain.getRiverValue(worldX, worldZ);
            if (riverDist < 0.04f && height < SEA_LEVEL) {
                if (height >= 1) {
                    chunk.setBlock(x, height, z,
                        (riverDist < 0.02f) ? BlockType::Clay : BlockType::Gravel);
                }
            }
        }
    }

    // Pass 2: carve caves
    m_terrain.carveCaves(chunk);

    // Pass 3: generate ores (with ExtremeHills flag for emerald)
    OreGenerator::generate(chunk, m_seed, hasExtremeHills);

    // Pass 4: generate structures (villages, temples, dungeons, mineshafts)
    StructureGenerator::generateStructures(chunk, m_seed, m_biomes, m_terrain);

    // Pass 5: generate trees
    StructureGenerator::generateTrees(chunk, m_seed, m_biomes, m_terrain);

    // Pass 6: generate vegetation (tall grass, flowers, cacti, snow, etc.)
    StructureGenerator::generateVegetation(chunk, m_seed, m_biomes, m_terrain);
}

} // namespace voxelforge
