#pragma once
#include <glad/gl.h>
#include <cstdint>

namespace voxelforge {

constexpr int ATLAS_SIZE    = 256;  // 256x256 pixels
constexpr int TILE_SIZE     = 16;   // each texture 16x16
constexpr int TILES_PER_ROW = ATLAS_SIZE / TILE_SIZE;

class TextureAtlas {
public:
    TextureAtlas() = default;
    ~TextureAtlas();

    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;

    void generate();
    void bind(int unit = 0) const;

    static void getUV(uint8_t textureIndex, float& u0, float& v0, float& u1, float& v1);

private:
    void fillTile(uint8_t* data, int tileX, int tileY,
                  uint8_t r, uint8_t g, uint8_t b);

    GLuint m_texture = 0;
};

} // namespace voxelforge
