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

class ChunkMesher {
public:
    static ChunkMesh generateMesh(const Chunk& chunk,
                                  const ChunkNeighbors& neighbors = {});

private:
    static BlockType sampleBlock(const Chunk& chunk, const ChunkNeighbors& nb,
                                 int x, int y, int z);
    static void addFace(ChunkMesh& mesh, const glm::vec3& blockPos,
                        BlockFace face, uint8_t texIdx, float brightness,
                        const float aoValues[4]);
};

} // namespace voxelforge
