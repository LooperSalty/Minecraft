#include "ChunkMesher.h"
#include "TextureAtlas.h"
#include "../world/Block.h"

namespace voxelforge {

// Per-face geometry: 4 vertex positions, UV mapping, normal, brightness.
// Winding is CCW when viewed from the outward normal direction.
struct FaceTemplate {
    float pos[4][3];
    float uv[4][2];
    float normal[3];
    float brightness;
};

static constexpr FaceTemplate FACES[6] = {
    // Top  Y+
    {{{0,1,0},{0,1,1},{1,1,1},{1,1,0}},
     {{0,0},{0,1},{1,1},{1,0}},
     {0,1,0}, 1.0f},
    // Bottom Y-
    {{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},
     {{0,0},{1,0},{1,1},{0,1}},
     {0,-1,0}, 0.5f},
    // North Z-
    {{{0,1,0},{1,1,0},{1,0,0},{0,0,0}},
     {{0,0},{1,0},{1,1},{0,1}},
     {0,0,-1}, 0.8f},
    // South Z+
    {{{0,0,1},{1,0,1},{1,1,1},{0,1,1}},
     {{0,1},{1,1},{1,0},{0,0}},
     {0,0,1}, 0.8f},
    // East  X+
    {{{1,1,0},{1,1,1},{1,0,1},{1,0,0}},
     {{0,0},{1,0},{1,1},{0,1}},
     {1,0,0}, 0.6f},
    // West  X-
    {{{0,1,1},{0,1,0},{0,0,0},{0,0,1}},
     {{0,0},{1,0},{1,1},{0,1}},
     {-1,0,0}, 0.6f},
};

static constexpr int NEIGHBOR[6][3] = {
    { 0, 1, 0}, { 0,-1, 0},
    { 0, 0,-1}, { 0, 0, 1},
    { 1, 0, 0}, {-1, 0, 0},
};

BlockType ChunkMesher::sampleBlock(const Chunk& chunk, const ChunkNeighbors& nb,
                                    int x, int y, int z) {
    if (y < 0 || y >= CHUNK_HEIGHT) return BlockType::Air;
    if (x < 0 && nb.negX) return nb.negX->getBlock(CHUNK_WIDTH + x, y, z);
    if (x >= CHUNK_WIDTH && nb.posX) return nb.posX->getBlock(x - CHUNK_WIDTH, y, z);
    if (z < 0 && nb.negZ) return nb.negZ->getBlock(x, y, CHUNK_DEPTH + z);
    if (z >= CHUNK_DEPTH && nb.posZ) return nb.posZ->getBlock(x, y, z - CHUNK_DEPTH);
    return chunk.getBlock(x, y, z);
}

ChunkMesh ChunkMesher::generateMesh(const Chunk& chunk,
                                     const ChunkNeighbors& neighbors) {
    ChunkMesh mesh;
    mesh.vertices.reserve(16384);
    mesh.indices.reserve(24576);

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                BlockType block = chunk.getBlock(x, y, z);
                if (block == BlockType::Air) continue;

                const auto& bd = getBlockData(block);
                glm::vec3 wp(chunk.toWorldPos(x, y, z));

                for (int f = 0; f < 6; ++f) {
                    int nx = x + NEIGHBOR[f][0];
                    int ny = y + NEIGHBOR[f][1];
                    int nz = z + NEIGHBOR[f][2];

                    if (!isBlockOpaque(sampleBlock(chunk, neighbors, nx, ny, nz))) {
                        addFace(mesh, wp, static_cast<BlockFace>(f),
                                bd.textureIndices[f], FACES[f].brightness);
                    }
                }
            }
        }
    }
    return mesh;
}

void ChunkMesher::addFace(ChunkMesh& mesh, const glm::vec3& pos,
                           BlockFace face, uint8_t texIdx, float brightness)
{
    int fi = static_cast<int>(face);
    const auto& ft = FACES[fi];

    float u0, v0, u1, v1;
    TextureAtlas::getUV(texIdx, u0, v0, u1, v1);

    uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

    for (int i = 0; i < 4; ++i) {
        Vertex vtx;
        vtx.x  = pos.x + ft.pos[i][0];
        vtx.y  = pos.y + ft.pos[i][1];
        vtx.z  = pos.z + ft.pos[i][2];
        vtx.u  = (ft.uv[i][0] < 0.5f) ? u0 : u1;
        vtx.v  = (ft.uv[i][1] < 0.5f) ? v0 : v1;
        vtx.light = brightness;
        vtx.nx = ft.normal[0];
        vtx.ny = ft.normal[1];
        vtx.nz = ft.normal[2];
        mesh.vertices.push_back(vtx);
    }

    mesh.indices.push_back(base + 0);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 0);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 3);
}

} // namespace voxelforge
