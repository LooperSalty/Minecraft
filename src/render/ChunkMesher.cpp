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

// AO corner offsets per face, per vertex
struct AOCornerOffsets {
    int side1[3];
    int side2[3];
    int corner[3];
};

static constexpr AOCornerOffsets AO_TOP[4] = {
    {{-1,1,0}, {0,1,-1}, {-1,1,-1}},
    {{-1,1,0}, {0,1,1},  {-1,1,1}},
    {{1,1,0},  {0,1,1},  {1,1,1}},
    {{1,1,0},  {0,1,-1}, {1,1,-1}},
};

static constexpr AOCornerOffsets AO_BOTTOM[4] = {
    {{-1,-1,0}, {0,-1,-1}, {-1,-1,-1}},
    {{1,-1,0},  {0,-1,-1}, {1,-1,-1}},
    {{1,-1,0},  {0,-1,1},  {1,-1,1}},
    {{-1,-1,0}, {0,-1,1},  {-1,-1,1}},
};

static constexpr AOCornerOffsets AO_NORTH[4] = {
    {{-1,0,-1}, {0,1,-1},  {-1,1,-1}},
    {{1,0,-1},  {0,1,-1},  {1,1,-1}},
    {{1,0,-1},  {0,-1,-1}, {1,-1,-1}},
    {{-1,0,-1}, {0,-1,-1}, {-1,-1,-1}},
};

static constexpr AOCornerOffsets AO_SOUTH[4] = {
    {{-1,0,1}, {0,-1,1}, {-1,-1,1}},
    {{1,0,1},  {0,-1,1}, {1,-1,1}},
    {{1,0,1},  {0,1,1},  {1,1,1}},
    {{-1,0,1}, {0,1,1},  {-1,1,1}},
};

static constexpr AOCornerOffsets AO_EAST[4] = {
    {{1,0,-1}, {1,1,0},  {1,1,-1}},
    {{1,0,1},  {1,1,0},  {1,1,1}},
    {{1,0,1},  {1,-1,0}, {1,-1,1}},
    {{1,0,-1}, {1,-1,0}, {1,-1,-1}},
};

static constexpr AOCornerOffsets AO_WEST[4] = {
    {{-1,0,1},  {-1,1,0},  {-1,1,1}},
    {{-1,0,-1}, {-1,1,0},  {-1,1,-1}},
    {{-1,0,-1}, {-1,-1,0}, {-1,-1,-1}},
    {{-1,0,1},  {-1,-1,0}, {-1,-1,1}},
};

static const AOCornerOffsets* AO_FACE_TABLE[6] = {
    AO_TOP, AO_BOTTOM, AO_NORTH, AO_SOUTH, AO_EAST, AO_WEST,
};

static constexpr float AO_LUT[4] = { 1.0f, 0.7f, 0.5f, 0.2f };

static float computeVertexAO(bool side1, bool side2, bool corner) {
    int count = 0;
    if (side1 && side2) {
        count = 3;
    } else {
        if (side1) ++count;
        if (side2) ++count;
        if (corner) ++count;
    }
    return AO_LUT[count];
}

BlockType ChunkMesher::sampleBlock(const Chunk& chunk, const ChunkNeighbors& nb,
                                    int x, int y, int z) {
    if (y < 0 || y >= CHUNK_HEIGHT) return BlockType::Air;
    if (x < 0 && nb.negX) return nb.negX->getBlock(CHUNK_WIDTH + x, y, z);
    if (x >= CHUNK_WIDTH && nb.posX) return nb.posX->getBlock(x - CHUNK_WIDTH, y, z);
    if (z < 0 && nb.negZ) return nb.negZ->getBlock(x, y, CHUNK_DEPTH + z);
    if (z >= CHUNK_DEPTH && nb.posZ) return nb.posZ->getBlock(x, y, z - CHUNK_DEPTH);
    return chunk.getBlock(x, y, z);
}

ChunkMeshPair ChunkMesher::generateMesh(const Chunk& chunk,
                                         const ChunkNeighbors& neighbors) {
    ChunkMeshPair result;
    result.opaque.vertices.reserve(16384);
    result.opaque.indices.reserve(24576);
    result.transparent.vertices.reserve(4096);
    result.transparent.indices.reserve(6144);

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                BlockType block = chunk.getBlock(x, y, z);
                if (block == BlockType::Air) continue;

                const auto& bd = getBlockData(block);
                glm::vec3 wp(chunk.toWorldPos(x, y, z));

                bool transparent = isBlockTransparent(block);
                bool isWater = (block == BlockType::Water);
                float waterFlag = isWater ? 1.0f : (transparent ? 0.5f : 0.0f);

                // Water surface is slightly lower
                float yScale = isWater ? 0.875f : 1.0f;

                ChunkMesh& targetMesh = transparent ? result.transparent : result.opaque;

                for (int f = 0; f < 6; ++f) {
                    int nx = x + NEIGHBOR[f][0];
                    int ny = y + NEIGHBOR[f][1];
                    int nz = z + NEIGHBOR[f][2];

                    BlockType neighborBlock = sampleBlock(chunk, neighbors, nx, ny, nz);

                    // For transparent blocks: don't cull face if neighbor is air
                    // or a different block type. Do cull if neighbor is same type.
                    bool shouldRender = false;
                    if (transparent) {
                        // Don't render face between same transparent blocks
                        shouldRender = (neighborBlock != block) && !isBlockOpaque(neighborBlock);
                        // Also render if neighbor is air
                        if (neighborBlock == BlockType::Air) shouldRender = true;
                    } else {
                        shouldRender = !isBlockOpaque(neighborBlock);
                    }

                    if (shouldRender) {
                        const AOCornerOffsets* aoOffsets = AO_FACE_TABLE[f];
                        float aoValues[4];

                        for (int v = 0; v < 4; ++v) {
                            bool s1 = isBlockOpaque(sampleBlock(chunk, neighbors,
                                x + aoOffsets[v].side1[0],
                                y + aoOffsets[v].side1[1],
                                z + aoOffsets[v].side1[2]));
                            bool s2 = isBlockOpaque(sampleBlock(chunk, neighbors,
                                x + aoOffsets[v].side2[0],
                                y + aoOffsets[v].side2[1],
                                z + aoOffsets[v].side2[2]));
                            bool cn = isBlockOpaque(sampleBlock(chunk, neighbors,
                                x + aoOffsets[v].corner[0],
                                y + aoOffsets[v].corner[1],
                                z + aoOffsets[v].corner[2]));
                            aoValues[v] = computeVertexAO(s1, s2, cn);
                        }

                        addFace(targetMesh, wp, static_cast<BlockFace>(f),
                                bd.textureIndices[f], FACES[f].brightness,
                                aoValues, waterFlag, yScale);
                    }
                }
            }
        }
    }
    return result;
}

void ChunkMesher::addFace(ChunkMesh& mesh, const glm::vec3& pos,
                           BlockFace face, uint8_t texIdx, float brightness,
                           const float aoValues[4], float waterFlag,
                           float yScale)
{
    int fi = static_cast<int>(face);
    const auto& ft = FACES[fi];

    float u0, v0, u1, v1;
    TextureAtlas::getUV(texIdx, u0, v0, u1, v1);

    uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

    for (int i = 0; i < 4; ++i) {
        Vertex vtx;
        vtx.x  = pos.x + ft.pos[i][0];
        vtx.y  = pos.y + ft.pos[i][1] * yScale;
        vtx.z  = pos.z + ft.pos[i][2];
        vtx.u  = (ft.uv[i][0] < 0.5f) ? u0 : u1;
        vtx.v  = (ft.uv[i][1] < 0.5f) ? v0 : v1;
        vtx.light = brightness;
        vtx.nx = ft.normal[0];
        vtx.ny = ft.normal[1];
        vtx.nz = ft.normal[2];
        vtx.ao = aoValues[i];
        vtx.isWater = waterFlag;
        mesh.vertices.push_back(vtx);
    }

    if (aoValues[0] + aoValues[2] > aoValues[1] + aoValues[3]) {
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    } else {
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 3);
        mesh.indices.push_back(base + 0);
    }
}

} // namespace voxelforge
