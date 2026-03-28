#pragma once
#include "../world/Chunk.h"
#include <vector>
#include <cstdint>

namespace voxelforge {

struct Vertex {
    float x, y, z;
    float u, v;
    float light;
    float nx, ny, nz;
    float ao;
    float isWater;  // 1.0 for water, 0.5 for glass/transparent, 0.0 for opaque
};

struct ChunkMesh {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    bool isEmpty() const { return vertices.empty(); }
};

struct ChunkNeighbors {
    const Chunk* negX = nullptr; // west
    const Chunk* posX = nullptr; // east
    const Chunk* negZ = nullptr; // north
    const Chunk* posZ = nullptr; // south
};

struct ChunkMeshPair {
    ChunkMesh opaque;
    ChunkMesh transparent;
};

class ChunkMesher {
public:
    static ChunkMeshPair generateMesh(const Chunk& chunk,
                                      const ChunkNeighbors& neighbors = {});

private:
    static BlockType sampleBlock(const Chunk& chunk, const ChunkNeighbors& nb,
                                 int x, int y, int z);
    static void addFace(ChunkMesh& mesh, const glm::vec3& blockPos,
                        BlockFace face, uint8_t texIdx, float brightness,
                        const float aoValues[4], float waterFlag,
                        float yScale = 1.0f);
};

} // namespace voxelforge
