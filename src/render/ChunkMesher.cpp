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

// AO corner offsets per face, per vertex (4 vertices x 2 side + 1 corner neighbor).
// For each face, each corner vertex has three neighbors that contribute to AO:
// two edge-adjacent blocks and one corner-diagonal block.
// Axes for each face:
//   Top(Y+):    tangent=X, bitangent=Z
//   Bottom(Y-): tangent=X, bitangent=Z
//   North(Z-):  tangent=X, bitangent=Y
//   South(Z+):  tangent=X, bitangent=Y
//   East(X+):   tangent=Z, bitangent=Y
//   West(X-):   tangent=Z, bitangent=Y

// For each face, for each of 4 vertices: offsets to 3 neighbors (side1, side2, corner)
// relative to the block position, along the face's normal + tangent/bitangent.
// side1, side2 are edge-adjacent, corner is the diagonal.
struct AOCornerOffsets {
    int side1[3];
    int side2[3];
    int corner[3];
};

// Top face (Y+): vertices are at y+1 plane
// v0=(0,1,0) v1=(0,1,1) v2=(1,1,1) v3=(1,1,0)
static constexpr AOCornerOffsets AO_TOP[4] = {
    // v0 (0,1,0) - corner at (-X, +Y, -Z)
    {{-1,1,0}, {0,1,-1}, {-1,1,-1}},
    // v1 (0,1,1) - corner at (-X, +Y, +Z)
    {{-1,1,0}, {0,1,1},  {-1,1,1}},
    // v2 (1,1,1) - corner at (+X, +Y, +Z)
    {{1,1,0},  {0,1,1},  {1,1,1}},
    // v3 (1,1,0) - corner at (+X, +Y, -Z)
    {{1,1,0},  {0,1,-1}, {1,1,-1}},
};

// Bottom face (Y-): vertices at y-1 plane
// v0=(0,0,0) v1=(1,0,0) v2=(1,0,1) v3=(0,0,1)
static constexpr AOCornerOffsets AO_BOTTOM[4] = {
    {{-1,-1,0}, {0,-1,-1}, {-1,-1,-1}},
    {{1,-1,0},  {0,-1,-1}, {1,-1,-1}},
    {{1,-1,0},  {0,-1,1},  {1,-1,1}},
    {{-1,-1,0}, {0,-1,1},  {-1,-1,1}},
};

// North face (Z-): vertices at z-1 plane
// v0=(0,1,0) v1=(1,1,0) v2=(1,0,0) v3=(0,0,0)
static constexpr AOCornerOffsets AO_NORTH[4] = {
    {{-1,0,-1}, {0,1,-1},  {-1,1,-1}},
    {{1,0,-1},  {0,1,-1},  {1,1,-1}},
    {{1,0,-1},  {0,-1,-1}, {1,-1,-1}},
    {{-1,0,-1}, {0,-1,-1}, {-1,-1,-1}},
};

// South face (Z+): vertices at z+1 plane
// v0=(0,0,1) v1=(1,0,1) v2=(1,1,1) v3=(0,1,1)
static constexpr AOCornerOffsets AO_SOUTH[4] = {
    {{-1,0,1}, {0,-1,1}, {-1,-1,1}},
    {{1,0,1},  {0,-1,1}, {1,-1,1}},
    {{1,0,1},  {0,1,1},  {1,1,1}},
    {{-1,0,1}, {0,1,1},  {-1,1,1}},
};

// East face (X+): vertices at x+1 plane
// v0=(1,1,0) v1=(1,1,1) v2=(1,0,1) v3=(1,0,0)
static constexpr AOCornerOffsets AO_EAST[4] = {
    {{1,0,-1}, {1,1,0},  {1,1,-1}},
    {{1,0,1},  {1,1,0},  {1,1,1}},
    {{1,0,1},  {1,-1,0}, {1,-1,1}},
    {{1,0,-1}, {1,-1,0}, {1,-1,-1}},
};

// West face (X-): vertices at x-1 plane
// v0=(0,1,1) v1=(0,1,0) v2=(0,0,0) v3=(0,0,1)
static constexpr AOCornerOffsets AO_WEST[4] = {
    {{-1,0,1},  {-1,1,0},  {-1,1,1}},
    {{-1,0,-1}, {-1,1,0},  {-1,1,-1}},
    {{-1,0,-1}, {-1,-1,0}, {-1,-1,-1}},
    {{-1,0,1},  {-1,-1,0}, {-1,-1,1}},
};

static const AOCornerOffsets* AO_FACE_TABLE[6] = {
    AO_TOP, AO_BOTTOM, AO_NORTH, AO_SOUTH, AO_EAST, AO_WEST,
};

// AO LUT: 0 neighbors occluding = 1.0, 1 = 0.7, 2 = 0.5, 3 = 0.2
static constexpr float AO_LUT[4] = { 1.0f, 0.7f, 0.5f, 0.2f };

// Compute vertex AO: count how many of side1, side2, corner are opaque.
// If both sides are opaque, the corner is always occluded (count=3).
static float computeVertexAO(bool side1, bool side2, bool corner) {
    int count = 0;
    if (side1 && side2) {
        count = 3; // Both sides block the corner
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
                        // Compute per-vertex AO for this face
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

                        addFace(mesh, wp, static_cast<BlockFace>(f),
                                bd.textureIndices[f], FACES[f].brightness,
                                aoValues);
                    }
                }
            }
        }
    }
    return mesh;
}

void ChunkMesher::addFace(ChunkMesh& mesh, const glm::vec3& pos,
                           BlockFace face, uint8_t texIdx, float brightness,
                           const float aoValues[4])
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
        vtx.ao = aoValues[i];
        mesh.vertices.push_back(vtx);
    }

    // Use flipped quad triangulation when AO is anisotropic to avoid
    // the diagonal artifact. If ao[0]+ao[2] > ao[1]+ao[3] use standard
    // triangulation (0-1-2, 0-2-3), otherwise flip to (1-2-3, 1-3-0).
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
