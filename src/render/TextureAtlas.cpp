#include "TextureAtlas.h"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace voxelforge {

TextureAtlas::~TextureAtlas() {
    if (m_texture) glDeleteTextures(1, &m_texture);
}

// --- Helpers ---

int TextureAtlas::hash(int x, int y, int seed) {
    int h = seed;
    h ^= x * 374761393;
    h ^= y * 668265263;
    h = (h ^ (h >> 13)) * 1274126177;
    return h;
}

float TextureAtlas::noise2D(int x, int y, int seed) {
    // Returns a value roughly in [0,1]
    return static_cast<float>((hash(x, y, seed) & 0xFFFF)) / 65535.0f;
}

void TextureAtlas::setPixel(uint8_t* data, int tileX, int tileY,
                             int px, int py,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    int bx = tileX * TILE_SIZE + px;
    int by = tileY * TILE_SIZE + py;
    int idx = (by * ATLAS_SIZE + bx) * 4;
    data[idx + 0] = r;
    data[idx + 1] = g;
    data[idx + 2] = b;
    data[idx + 3] = a;
}

void TextureAtlas::fillTile(uint8_t* data, int tileX, int tileY,
                             uint8_t r, uint8_t g, uint8_t b)
{
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int n = static_cast<int>((noise2D(x, y, tileX * 17 + tileY * 31) - 0.5f) * 20.0f);
            setPixel(data, tileX, tileY, x, y,
                     static_cast<uint8_t>(std::clamp(int(r) + n, 0, 255)),
                     static_cast<uint8_t>(std::clamp(int(g) + n, 0, 255)),
                     static_cast<uint8_t>(std::clamp(int(b) + n, 0, 255)));
        }
    }
}

void TextureAtlas::generate() {
    std::vector<uint8_t> px(ATLAS_SIZE * ATLAS_SIZE * 4, 255);
    std::srand(42);

    // Lambda helpers for cleaner code
    auto clampByte = [](int v) -> uint8_t {
        return static_cast<uint8_t>(std::clamp(v, 0, 255));
    };

    auto setpx = [&](int tileX, int tileY, int x, int y,
                      int r, int g, int b, int a = 255) {
        setPixel(px.data(), tileX, tileY, x, y,
                 clampByte(r), clampByte(g), clampByte(b), clampByte(a));
    };

    // ================================================================
    //  0 Stone - Gray with darker specks and subtle cracks
    // ================================================================
    {
        int tx = 0, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 100);
                int base = 128 + static_cast<int>((n - 0.5f) * 20.0f);
                // Darker specks
                float speck = noise2D(x * 3, y * 7, 101);
                if (speck > 0.8f) base -= 25;
                // Subtle cracks: horizontal and vertical lines
                float crack = noise2D(x, y * 5, 102);
                if (crack > 0.92f) base -= 35;
                float crack2 = noise2D(x * 5, y, 103);
                if (crack2 > 0.93f) base -= 30;
                setpx(tx, ty, x, y, base, base, base);
            }
        }
    }

    // ================================================================
    //  1 Dirt - Brown with darker brown spots and tiny pebbles
    // ================================================================
    {
        int tx = 1, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 200);
                int r = 134, g = 96, b = 67;
                int variation = static_cast<int>((n - 0.5f) * 24.0f);
                r += variation; g += variation; b += variation;
                // Darker spots
                float spot = noise2D(x * 3, y * 3, 201);
                if (spot > 0.75f) { r -= 20; g -= 15; b -= 10; }
                // Tiny pebbles (lighter pixels)
                float pebble = noise2D(x * 7, y * 11, 202);
                if (pebble > 0.9f) { r += 25; g += 20; b += 15; }
                setpx(tx, ty, x, y, r, g, b);
            }
        }
    }

    // ================================================================
    //  2 Grass top - Bright green with darker green variation
    // ================================================================
    {
        int tx = 2, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 300);
                float n2 = noise2D(x * 2 + 5, y * 3 + 7, 301);
                int r = 89, g = 150, b = 56;
                int variation = static_cast<int>((n - 0.5f) * 30.0f);
                r += variation; g += variation; b += variation;
                // Darker patches
                if (n2 > 0.7f) { r -= 18; g -= 12; b -= 15; }
                // Bright highlight pixels
                float hi = noise2D(x * 5, y * 5, 302);
                if (hi > 0.88f) { r += 15; g += 20; b += 8; }
                setpx(tx, ty, x, y, r, g, b);
            }
        }
    }

    // ================================================================
    //  3 Grass side - Dirt with green fringe at top (2-4 pixels)
    // ================================================================
    {
        int tx = 3, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 400);
                int variation = static_cast<int>((n - 0.5f) * 18.0f);
                // Green fringe at top with jagged edge
                float edgeNoise = noise2D(x * 3, 0, 401);
                int greenDepth = 2 + static_cast<int>(edgeNoise * 3.0f);
                if (y < greenDepth) {
                    int r = 89 + variation, g = 150 + variation, b = 56 + variation;
                    // Transition row: blend
                    if (y == greenDepth - 1) {
                        r = (r + 134) / 2; g = (g + 96) / 2; b = (b + 67) / 2;
                    }
                    setpx(tx, ty, x, y, r, g, b);
                } else {
                    int r = 134 + variation, g = 96 + variation, b = 67 + variation;
                    float spot = noise2D(x * 3, y * 3, 402);
                    if (spot > 0.75f) { r -= 18; g -= 13; b -= 9; }
                    setpx(tx, ty, x, y, r, g, b);
                }
            }
        }
    }

    // ================================================================
    //  4 Cobblestone - Gray stones with darker mortar lines
    // ================================================================
    {
        int tx = 4, ty = 0;
        // Create a stone pattern with mortar
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 500);
                // Mortar grid: every ~4 pixels, offset rows
                int row = y / 4;
                int offset = (row % 2) * 2;
                int col = (x + offset) / 5;
                bool mortar = ((y % 4) == 0) || (((x + offset) % 5) == 0 && (y % 4) != 0);
                int variation = static_cast<int>((n - 0.5f) * 18.0f);
                if (mortar) {
                    // Dark mortar lines
                    int v = 80 + variation;
                    setpx(tx, ty, x, y, v, v, v);
                } else {
                    // Stone: vary per cell for color variation
                    float cellN = noise2D(col * 7, row * 13, 501);
                    int base = 120 + static_cast<int>((cellN - 0.5f) * 40.0f);
                    int v = base + variation;
                    setpx(tx, ty, x, y, v, v, v);
                }
            }
        }
    }

    // ================================================================
    //  5 Oak log top - Concentric brown rings
    // ================================================================
    {
        int tx = 5, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float dx = x - 7.5f, dy = y - 7.5f;
                float d = std::sqrt(dx * dx + dy * dy);
                float n = noise2D(x, y, 600) * 0.3f;
                bool ring = static_cast<int>(d + n * 2.0f) % 3 == 0;
                int variation = static_cast<int>((noise2D(x * 3, y * 3, 601) - 0.5f) * 12.0f);
                if (ring) {
                    setpx(tx, ty, x, y, 120 + variation, 85 + variation, 45 + variation);
                } else {
                    setpx(tx, ty, x, y, 165 + variation, 130 + variation, 75 + variation);
                }
                // Dark outer edge (bark ring)
                if (d > 6.5f) {
                    setpx(tx, ty, x, y, 90 + variation, 65 + variation, 30 + variation);
                }
            }
        }
    }

    // ================================================================
    //  6 Oak log side - Brown bark with vertical grain lines
    // ================================================================
    {
        int tx = 6, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 700);
                int variation = static_cast<int>((n - 0.5f) * 16.0f);
                int r = 102, g = 76, b = 40;
                // Vertical grain lines
                float grain = noise2D(x * 5, y, 701);
                if (grain > 0.7f) { r -= 15; g -= 12; b -= 8; }
                // Bark texture: horizontal cracks
                float bark = noise2D(x, y * 7, 702);
                if (bark > 0.85f) { r -= 20; g -= 16; b -= 10; }
                // Lighter vertical streaks
                if (x % 5 == 2) { r += 10; g += 8; b += 5; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    //  7 Oak planks - Warm brown with horizontal wood grain
    // ================================================================
    {
        int tx = 7, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 800);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 180, g = 144, b = 90;
                // Horizontal grain lines
                float grain = noise2D(x, y * 5, 801);
                if (grain > 0.75f) { r -= 12; g -= 10; b -= 6; }
                // Board separation: horizontal line every 4 pixels
                if (y % 8 == 0) { r -= 25; g -= 20; b -= 14; }
                // Slight color shift between boards
                int board = y / 8;
                float boardN = noise2D(board * 11, 0, 802);
                int boardShift = static_cast<int>((boardN - 0.5f) * 16.0f);
                r += boardShift; g += boardShift; b += boardShift;
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    //  8 Bedrock - Very dark gray/black with scattered lighter spots
    // ================================================================
    {
        int tx = 8, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 900);
                int base = 35 + static_cast<int>((n - 0.5f) * 20.0f);
                // Lighter gray spots
                float spot = noise2D(x * 5, y * 3, 901);
                if (spot > 0.8f) base += 30;
                float spot2 = noise2D(x * 3, y * 7, 902);
                if (spot2 > 0.85f) base += 20;
                setpx(tx, ty, x, y, base, base, base);
            }
        }
    }

    // ================================================================
    //  9 Sand - Pale yellow with slightly darker speckles
    // ================================================================
    {
        int tx = 9, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1000);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                int r = 219, g = 211, b = 160;
                // Darker speckles
                float speck = noise2D(x * 7, y * 7, 1001);
                if (speck > 0.78f) { r -= 15; g -= 12; b -= 8; }
                // Very slight grain
                float grain = noise2D(x * 2 + y, y * 2, 1002);
                int gv = static_cast<int>((grain - 0.5f) * 8.0f);
                setpx(tx, ty, x, y, r + variation + gv, g + variation + gv, b + variation + gv);
            }
        }
    }

    // ================================================================
    // 10 Gravel
    // ================================================================
    {
        int tx = 10, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1100);
                int base = 130 + static_cast<int>((n - 0.5f) * 30.0f);
                // Pebble variation
                float peb = noise2D(x * 3, y * 3, 1101);
                if (peb > 0.6f) base += 15;
                if (peb < 0.3f) base -= 15;
                setpx(tx, ty, x, y, base, base - 4, base - 8);
            }
        }
    }

    // ================================================================
    // 11 Water - Blue semi-transparent with wave pattern
    // ================================================================
    {
        int tx = 11, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1200);
                float wave = noise2D(x * 2 + y, y * 2 - x, 1201);
                int r = 25, g = 70, b = 180;
                int variation = static_cast<int>((n - 0.5f) * 18.0f);
                // Wave highlights
                if (wave > 0.75f) { r += 15; g += 25; b += 20; }
                // Subtle darker ripples
                float ripple = noise2D(x * 3, y * 5, 1202);
                if (ripple > 0.85f) { r -= 8; g -= 5; b += 10; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation, 155);
            }
        }
    }

    // ================================================================
    // 12 Lava
    // ================================================================
    {
        int tx = 12, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1300);
                float n2 = noise2D(x * 2, y * 2, 1301);
                int r = 207, g = 91, b = 20;
                int v = static_cast<int>((n - 0.5f) * 30.0f);
                r += v; g += v / 2; b += v / 3;
                // Hot spots
                if (n2 > 0.8f) { r += 30; g += 40; b += 15; }
                // Dark crust
                if (n2 < 0.2f) { r -= 50; g -= 30; b -= 10; }
                setpx(tx, ty, x, y, r, g, b);
            }
        }
    }

    // ================================================================
    // 13 OakLeaves - Dark green with alpha-cutout holes
    // ================================================================
    {
        int tx = 13, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1400);
                float hole = noise2D(x * 5, y * 5, 1401);
                int variation = static_cast<int>((n - 0.5f) * 35.0f);
                // Leaf cutout holes: fully transparent so shader discards
                if (hole > 0.82f) {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                } else {
                    int r = 56 + variation, g = 118 + variation, b = 29 + variation;
                    float hi = noise2D(x * 3, y * 3, 1402);
                    if (hi > 0.7f) { r += 15; g += 20; b += 10; }
                    setpx(tx, ty, x, y, r, g, b, 255);
                }
            }
        }
    }

    // ================================================================
    // 14 BirchLog side
    // ================================================================
    {
        int tx = 14, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1500);
                int variation = static_cast<int>((n - 0.5f) * 8.0f);
                int base = 200;
                // Dark horizontal bands (birch bark)
                bool band = (y % 5 == 0) || (y % 5 == 1 && x % 3 == 0);
                float bandN = noise2D(x * 3, y, 1501);
                if (band && bandN > 0.4f) base -= 50;
                // Vertical streaks
                if (x % 4 == 0 || x % 7 == 0) {
                    float streak = noise2D(x, y * 3, 1502);
                    if (streak > 0.5f) base -= 30;
                }
                setpx(tx, ty, x, y, base + variation, base + variation, base - 5 + variation);
            }
        }
    }

    // ================================================================
    // 15 BirchLog top
    // ================================================================
    {
        int tx = 15, ty = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float dx = x - 7.5f, dy = y - 7.5f;
                float d = std::sqrt(dx * dx + dy * dy);
                int n = static_cast<int>((noise2D(x, y, 1600) - 0.5f) * 10.0f);
                bool ring = static_cast<int>(d) % 3 == 0;
                uint8_t base = ring ? 160 : 190;
                setpx(tx, ty, x, y, base + n, base - 10 + n, base - 40 + n);
                if (d > 6.5f) setpx(tx, ty, x, y, 180 + n, 180 + n, 175 + n);
            }
        }
    }

    // ================================================================
    // 16 BirchLeaves (index 16 → col=0, row=1 with 16-col layout)
    // ================================================================
    {
        int tx = 0, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1700);
                int variation = static_cast<int>((n - 0.5f) * 28.0f);
                float hole = noise2D(x * 5, y * 5, 1701);
                if (hole > 0.82f) {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                } else {
                    setpx(tx, ty, x, y, 100 + variation, 160 + variation, 70 + variation, 255);
                }
            }
        }
    }

    // ================================================================
    // Row 1 (tileY=1): 17=SpruceLog side, 18=SpruceLeaves, 19-25=ores, 26=snow, 27=sandstone
    // ================================================================

    // 17 SpruceLog side
    {
        int tx = 1, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1800);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 70, g = 45, b = 20;
                float grain = noise2D(x * 5, y, 1801);
                if (grain > 0.7f) { r -= 12; g -= 8; b -= 5; }
                if (x % 4 == 1) { r += 8; g += 5; b += 3; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // 18 SpruceLeaves
    {
        int tx = 2, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 1900);
                int variation = static_cast<int>((n - 0.5f) * 20.0f);
                float hole = noise2D(x * 5, y * 5, 1901);
                if (hole > 0.8f) {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                } else {
                    setpx(tx, ty, x, y, 30 + variation, 80 + variation, 25 + variation, 255);
                }
            }
        }
    }

    // Ore helper: stone base with clusters of colored ore specks
    auto fillOreTile = [&](int tileX, int tileY,
                           int sr, int sg, int sb, int oreSeed) {
        // Stone base
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, oreSeed);
                int base = 128 + static_cast<int>((n - 0.5f) * 20.0f);
                float speck = noise2D(x * 3, y * 7, oreSeed + 1);
                if (speck > 0.8f) base -= 20;
                setpx(tileX, tileY, x, y, base, base, base);
            }
        }
        // Ore clusters: 3-4 clusters of 2-4 pixels each
        for (int cluster = 0; cluster < 4; ++cluster) {
            int cx = static_cast<int>(noise2D(cluster, 0, oreSeed + 10) * 13.0f) + 1;
            int cy = static_cast<int>(noise2D(0, cluster, oreSeed + 11) * 13.0f) + 1;
            int size = 2 + static_cast<int>(noise2D(cluster, cluster, oreSeed + 12) * 2.0f);
            for (int dy = 0; dy < size && cy + dy < TILE_SIZE; ++dy) {
                for (int dx = 0; dx < size && cx + dx < TILE_SIZE; ++dx) {
                    float skip = noise2D(cx + dx, cy + dy, oreSeed + 13);
                    if (skip > 0.7f) continue; // irregular shape
                    int n = static_cast<int>((noise2D(cx + dx, cy + dy, oreSeed + 14) - 0.5f) * 20.0f);
                    setpx(tileX, tileY, cx + dx, cy + dy,
                           clampByte(sr + n), clampByte(sg + n), clampByte(sb + n));
                }
            }
        }
    };

    // 19 CoalOre
    fillOreTile(3, 1, 30, 30, 30, 2000);
    // 20 IronOre
    fillOreTile(4, 1, 186, 150, 115, 2100);
    // 21 GoldOre
    fillOreTile(5, 1, 230, 210, 60, 2200);
    // 22 DiamondOre
    fillOreTile(6, 1, 80, 220, 230, 2300);
    // 23 RedstoneOre
    fillOreTile(7, 1, 200, 30, 30, 2400);
    // 24 LapisOre
    fillOreTile(8, 1, 30, 50, 180, 2500);
    // 25 EmeraldOre
    fillOreTile(9, 1, 30, 190, 60, 2600);

    // ================================================================
    // 26 SnowBlock
    // ================================================================
    {
        int tx = 10, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 2700);
                int v = static_cast<int>((n - 0.5f) * 6.0f);
                setpx(tx, ty, x, y, 240 + v, 245 + v, 250 + v);
            }
        }
    }

    // ================================================================
    // 27 Sandstone
    // ================================================================
    {
        int tx = 11, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 2800);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 210, g = 195, b = 150;
                // Horizontal layers
                if (y % 4 == 0) { r -= 12; g -= 10; b -= 8; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 28 TallGrass
    // ================================================================
    {
        int tx = 12, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                bool blade = (x % 3 == 0) || (x % 5 == 1);
                int maxH = 4 + (8 - std::abs(x - 8));
                bool inBlade = blade && (y >= (TILE_SIZE - maxH));
                if (inBlade) {
                    float n = noise2D(x, y, 2900);
                    int variation = static_cast<int>((n - 0.5f) * 28.0f);
                    setpx(tx, ty, x, y, 60 + variation, 130 + variation, 40 + variation);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 29 Poppy
    // ================================================================
    {
        int tx = 13, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int n = static_cast<int>((noise2D(x, y, 3000) - 0.5f) * 10.0f);
                bool stem = (x >= 7 && x <= 8) && (y >= 8);
                float dx = x - 7.5f, dy = y - 4.5f;
                bool flower = (dx * dx + dy * dy) < 12.0f;
                if (flower) {
                    setpx(tx, ty, x, y, 200 + n, 30 + n, 30 + n);
                } else if (stem) {
                    setpx(tx, ty, x, y, 40 + n, 100 + n, 30 + n);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 30 Dandelion
    // ================================================================
    {
        int tx = 14, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int n = static_cast<int>((noise2D(x, y, 3100) - 0.5f) * 10.0f);
                bool stem = (x >= 7 && x <= 8) && (y >= 8);
                float dx = x - 7.5f, dy = y - 5.0f;
                bool flower = (dx * dx + dy * dy) < 10.0f;
                if (flower) {
                    setpx(tx, ty, x, y, 240 + n, 220 + n, 40 + n);
                } else if (stem) {
                    setpx(tx, ty, x, y, 40 + n, 100 + n, 30 + n);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 31 Cactus
    // ================================================================
    {
        int tx = 15, ty = 1;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3200);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 30, g = 100, b = 30;
                bool spot = ((x + y * 3) % 7 == 0) || ((x * 5 + y) % 11 == 0);
                if (spot) { r += 30; g += 40; b += 20; }
                if (x % 4 == 0) { r -= 10; g -= 15; b -= 10; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 32 Clay
    // ================================================================
    {
        int tx = 0, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3300);
                int variation = static_cast<int>((n - 0.5f) * 10.0f);
                setpx(tx, ty, x, y, 160 + variation, 155 + variation, 148 + variation);
            }
        }
    }

    // ================================================================
    // 33 Glass - Nearly transparent with thin white edge highlights
    // ================================================================
    {
        int tx = 1, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                // Mostly transparent with slight blue tint
                bool edge = (x == 0 || x == 15 || y == 0 || y == 15);
                bool innerEdge = (x == 1 || x == 14 || y == 1 || y == 14);
                if (edge) {
                    setpx(tx, ty, x, y, 200, 210, 220, 180);
                } else if (innerEdge) {
                    setpx(tx, ty, x, y, 180, 200, 210, 80);
                } else {
                    // Interior: very transparent with subtle blue
                    float n = noise2D(x, y, 3400);
                    int v = static_cast<int>(n * 10.0f);
                    setpx(tx, ty, x, y, 180 + v, 210 + v, 230 + v, 30);
                }
            }
        }
    }

    // ================================================================
    // 34 Bricks - Red-orange brick pattern with mortar
    // ================================================================
    {
        int tx = 2, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3500);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                // Brick rows: 4 pixels high, offset every other row
                int row = y / 4;
                int offset = (row % 2) * 4;
                int col = (x + offset) % 8;
                bool mortar = (y % 4 == 0) || (col == 0);
                if (mortar) {
                    // Light gray mortar
                    int mv = 195 + variation;
                    setpx(tx, ty, x, y, mv, mv - 5, mv - 10);
                } else {
                    // Red-orange brick with per-brick variation
                    float brickN = noise2D(col / 4, row, 3501);
                    int brickShift = static_cast<int>((brickN - 0.5f) * 20.0f);
                    setpx(tx, ty, x, y,
                           150 + variation + brickShift,
                           80 + variation / 2 + brickShift / 2,
                           60 + variation / 3 + brickShift / 3);
                }
            }
        }
    }

    // ================================================================
    // 35 Bookshelf sides - Books with colored spines
    // ================================================================
    {
        int tx = 3, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3600);
                int variation = static_cast<int>((n - 0.5f) * 8.0f);
                // Two rows of books: y 1-6 and y 9-14, with shelf at y 0,7,8,15
                bool shelf = (y == 0 || y == 7 || y == 8 || y == 15);
                if (shelf) {
                    // Dark brown shelf
                    setpx(tx, ty, x, y, 120 + variation, 90 + variation, 55 + variation);
                } else {
                    // Books: each book is 2-3 pixels wide, different colors
                    int bookIdx = x / 3;
                    float bookColor = noise2D(bookIdx, y / 8, 3601);
                    int bookR, bookG, bookB;
                    int bookVariant = static_cast<int>(bookColor * 6.0f) % 6;
                    switch (bookVariant) {
                        case 0: bookR = 140; bookG = 40; bookB = 40; break;  // Red
                        case 1: bookR = 40; bookG = 80; bookB = 140; break;   // Blue
                        case 2: bookR = 50; bookG = 120; bookB = 50; break;   // Green
                        case 3: bookR = 140; bookG = 120; bookB = 40; break;  // Yellow/tan
                        case 4: bookR = 100; bookG = 50; bookB = 120; break;  // Purple
                        default: bookR = 80; bookG = 80; bookB = 80; break;   // Gray
                    }
                    // Book spine highlight (leftmost pixel of each book)
                    if (x % 3 == 0) {
                        bookR += 20; bookG += 20; bookB += 20;
                    }
                    setpx(tx, ty, x, y, bookR + variation, bookG + variation, bookB + variation);
                }
            }
        }
    }

    // ================================================================
    // 36 TNT side - Red with dark stripes
    // ================================================================
    {
        int tx = 4, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3700);
                int variation = static_cast<int>((n - 0.5f) * 10.0f);
                // Red base with dark horizontal stripes
                int r = 190, g = 40, b = 35;
                // Top and bottom bands
                if (y < 2 || y >= 14) {
                    r = 80; g = 60; b = 40; // Dark brown band
                }
                // Middle lighter band with "TNT" area
                if (y >= 5 && y <= 10) {
                    r = 210; g = 180; b = 150; // Tan label area
                }
                // Vertical dark stripes on red parts
                if ((y < 5 || y > 10) && x % 4 == 0) {
                    r -= 30; g -= 10; b -= 10;
                }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 37 TNT top/bottom - Gray cap
    // ================================================================
    {
        int tx = 5, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3800);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                // Gray with darker center circle (fuse hole)
                float dx = x - 7.5f, dy = y - 7.5f;
                float d = std::sqrt(dx * dx + dy * dy);
                int base = 140;
                if (d < 3.0f) base = 60; // Dark fuse hole
                else if (d < 4.0f) base = 100; // Ring
                setpx(tx, ty, x, y, base + variation, base - 5 + variation, base - 10 + variation);
            }
        }
    }

    // ================================================================
    // 38 Pumpkin side - Orange with vertical ridges
    // ================================================================
    {
        int tx = 6, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 3900);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 200, g = 120, b = 20;
                // Vertical ridges every 4 pixels
                if (x % 4 == 0) { r -= 35; g -= 25; b -= 10; }
                if (x % 4 == 1) { r -= 15; g -= 10; b -= 5; }
                // Subtle horizontal gradient (darker at bottom)
                int gy = y / 2;
                r -= gy; g -= gy / 2;
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 39 Pumpkin top - Dark green stem on orange
    // ================================================================
    {
        int tx = 7, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4000);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                // Stem in center 4x4
                bool stem = (x >= 6 && x <= 9 && y >= 6 && y <= 9);
                if (stem) {
                    setpx(tx, ty, x, y, 50 + variation, 90 + variation, 20 + variation);
                } else {
                    // Orange top with radial ridges
                    float dx = x - 7.5f, dy = y - 7.5f;
                    float angle = std::atan2(dy, dx);
                    bool ridge = static_cast<int>((angle + 3.15f) * 2.5f) % 2 == 0;
                    int r = 200, g = 120, b = 20;
                    if (ridge) { r -= 20; g -= 15; b -= 5; }
                    setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
                }
            }
        }
    }

    // ================================================================
    // 40 Melon side - Green with dark green stripes
    // ================================================================
    {
        int tx = 8, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4100);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 100, g = 150, b = 30;
                // Vertical dark stripes
                if (x % 4 == 0 || x % 4 == 1) { r -= 30; g -= 25; b -= 10; }
                // Bottom slightly darker
                if (y > 12) { r -= 10; g -= 10; b -= 5; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 41 Melon top - Tan interior
    // ================================================================
    {
        int tx = 9, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4200);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                // Green edge, tan interior
                bool edge = (x < 2 || x > 13 || y < 2 || y > 13);
                if (edge) {
                    setpx(tx, ty, x, y, 80 + variation, 130 + variation, 30 + variation);
                } else {
                    // Tan/yellow melon flesh
                    setpx(tx, ty, x, y, 200 + variation, 180 + variation, 100 + variation);
                }
            }
        }
    }

    // ================================================================
    // 42 CraftingTable top - Dark planks with tool marks
    // ================================================================
    {
        int tx = 10, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4300);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                int r = 140, g = 105, b = 65;
                // Grid pattern (4x4 squares)
                if (x == 0 || x == 8 || y == 0 || y == 8) { r -= 30; g -= 25; b -= 15; }
                // Tool marks: darker scratches
                float mark = noise2D(x * 7, y * 3, 4301);
                if (mark > 0.88f) { r -= 25; g -= 20; b -= 12; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 43 CraftingTable side - Regular planks with slight variation
    // ================================================================
    {
        int tx = 11, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4400);
                int variation = static_cast<int>((n - 0.5f) * 14.0f);
                int r = 155, g = 120, b = 75;
                // Horizontal grain
                float grain = noise2D(x, y * 5, 4401);
                if (grain > 0.75f) { r -= 10; g -= 8; b -= 5; }
                // Board separation
                if (y % 8 == 0) { r -= 20; g -= 16; b -= 10; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 44 Furnace face - Stone with dark opening
    // ================================================================
    {
        int tx = 12, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4500);
                int variation = static_cast<int>((n - 0.5f) * 16.0f);
                // Stone base
                int base = 128;
                // Furnace opening (center rectangle)
                bool opening = (x >= 4 && x <= 11 && y >= 5 && y <= 12);
                bool rim = (x >= 3 && x <= 12 && y >= 4 && y <= 13) && !opening;
                if (opening) {
                    // Dark interior
                    setpx(tx, ty, x, y, 25 + variation / 2, 20 + variation / 2, 20 + variation / 2);
                } else if (rim) {
                    // Darker stone rim
                    setpx(tx, ty, x, y, 90 + variation, 90 + variation, 90 + variation);
                } else {
                    // Regular stone
                    float speck = noise2D(x * 3, y * 7, 4501);
                    if (speck > 0.8f) base -= 20;
                    setpx(tx, ty, x, y, base + variation, base + variation, base + variation);
                }
            }
        }
    }

    // ================================================================
    // 45 MossyCobblestone - Cobblestone with green moss patches
    // ================================================================
    {
        int tx = 13, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4600);
                float moss = noise2D(x * 3, y * 3, 4601);
                int variation = static_cast<int>((n - 0.5f) * 20.0f);
                if (moss > 0.55f) {
                    setpx(tx, ty, x, y, 60 + variation, 120 + variation, 50 + variation);
                } else {
                    setpx(tx, ty, x, y, 110 + variation, 110 + variation, 110 + variation);
                }
            }
        }
    }

    // ================================================================
    // 46 Ice - Light blue, semi-transparent
    // ================================================================
    {
        int tx = 14, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4700);
                float crack = noise2D(x * 4, y * 2, 4701);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                int a = 180;
                if (crack > 0.88f) {
                    setpx(tx, ty, x, y, 200 + variation, 230 + variation, 255, 220);
                } else {
                    setpx(tx, ty, x, y, 160 + variation, 200 + variation, 240 + variation, a);
                }
            }
        }
    }

    // ================================================================
    // 47 Mycelium top - Purple-gray fungal surface
    // ================================================================
    {
        int tx = 15, ty = 2;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4800);
                float spore = noise2D(x * 5, y * 5, 4801);
                int variation = static_cast<int>((n - 0.5f) * 20.0f);
                int r = 130, g = 100, b = 140;
                if (spore > 0.75f) { r += 20; g -= 10; b += 25; }
                setpx(tx, ty, x, y, r + variation, g + variation, b + variation);
            }
        }
    }

    // ================================================================
    // 48 Mycelium side - Purple-gray strip on top, dirt below
    // ================================================================
    {
        int tx = 0, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 4900);
                int variation = static_cast<int>((n - 0.5f) * 16.0f);
                if (y < 4) {
                    setpx(tx, ty, x, y, 130 + variation, 100 + variation, 140 + variation);
                } else {
                    setpx(tx, ty, x, y, 134 + variation, 96 + variation, 67 + variation);
                }
            }
        }
    }

    // ================================================================
    // 49 Vine - Dark green hanging plant, partially transparent
    // ================================================================
    {
        int tx = 1, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5000);
                bool leaf = (x % 3 != 2) && ((x + y) % 4 != 0);
                if (leaf) {
                    int variation = static_cast<int>((n - 0.5f) * 20.0f);
                    setpx(tx, ty, x, y, 30 + variation, 90 + variation, 20 + variation, 220);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 50 DeadBush - Brown sticks on transparent background
    // ================================================================
    {
        int tx = 2, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5100);
                int variation = static_cast<int>((n - 0.5f) * 10.0f);
                // Main stem and branches
                bool stem = (x >= 7 && x <= 8) && (y >= 6);
                bool branchL = (x < 8 && std::abs(x - 7) == (y - 3) && y >= 3 && y <= 8);
                bool branchR = (x > 7 && std::abs(x - 8) == (y - 4) && y >= 4 && y <= 9);
                if (stem || branchL || branchR) {
                    setpx(tx, ty, x, y, 120 + variation, 85 + variation, 40 + variation);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 51 LilyPad - Flat green circle on transparent
    // ================================================================
    {
        int tx = 3, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float dx = x - 7.5f, dy = y - 7.5f;
                bool pad = (dx * dx + dy * dy) < 50.0f;
                if (pad) {
                    float n = noise2D(x, y, 5200);
                    int variation = static_cast<int>((n - 0.5f) * 16.0f);
                    bool vein = (std::abs(x - 8) <= 1 && y < 8) || (std::abs(y - 8) <= 1 && x < 8);
                    if (vein) {
                        setpx(tx, ty, x, y, 20 + variation, 100 + variation, 15 + variation);
                    } else {
                        setpx(tx, ty, x, y, 40 + variation, 130 + variation, 25 + variation);
                    }
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 52 MushroomRed - Red cap with white spots on stem
    // ================================================================
    {
        int tx = 4, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5300);
                int variation = static_cast<int>((n - 0.5f) * 10.0f);
                bool stem = (x >= 6 && x <= 9) && (y >= 10);
                float cdx = x - 7.5f, cdy = y - 6.0f;
                bool cap = (cdx * cdx + cdy * cdy * 1.5f) < 25.0f && y < 11;
                bool spot = cap && ((x == 5 && y == 5) || (x == 10 && y == 5) ||
                                    (x == 7 && y == 3) || (x == 9 && y == 7));
                if (spot) {
                    setpx(tx, ty, x, y, 240 + variation, 240 + variation, 230 + variation);
                } else if (cap) {
                    setpx(tx, ty, x, y, 200 + variation, 30 + variation, 30 + variation);
                } else if (stem) {
                    setpx(tx, ty, x, y, 200 + variation, 190 + variation, 175 + variation);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 53 MushroomBrown - Brown cap on stem
    // ================================================================
    {
        int tx = 5, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5400);
                int variation = static_cast<int>((n - 0.5f) * 10.0f);
                bool stem = (x >= 6 && x <= 9) && (y >= 10);
                float cdx = x - 7.5f, cdy = y - 7.0f;
                bool cap = (cdx * cdx + cdy * cdy * 2.0f) < 30.0f && y < 11;
                if (cap) {
                    setpx(tx, ty, x, y, 140 + variation, 100 + variation, 60 + variation);
                } else if (stem) {
                    setpx(tx, ty, x, y, 200 + variation, 190 + variation, 175 + variation);
                } else {
                    setpx(tx, ty, x, y, 0, 0, 0, 0);
                }
            }
        }
    }

    // ================================================================
    // 54 ChestBlock top/bottom - Dark wood with metal latch
    // ================================================================
    {
        int tx = 6, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5500);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                bool latch = (x >= 6 && x <= 9 && y >= 6 && y <= 9);
                if (latch) {
                    setpx(tx, ty, x, y, 180 + variation, 180 + variation, 160 + variation);
                } else {
                    setpx(tx, ty, x, y, 140 + variation, 100 + variation, 50 + variation);
                }
            }
        }
    }

    // ================================================================
    // 55 ChestBlock side - Wood with dark border and latch
    // ================================================================
    {
        int tx = 7, ty = 3;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                float n = noise2D(x, y, 5600);
                int variation = static_cast<int>((n - 0.5f) * 12.0f);
                bool border = (x == 0 || x == 15 || y == 0 || y == 15);
                bool latch = (x >= 7 && x <= 8 && y >= 5 && y <= 8);
                if (border) {
                    setpx(tx, ty, x, y, 80 + variation, 60 + variation, 30 + variation);
                } else if (latch) {
                    setpx(tx, ty, x, y, 180 + variation, 180 + variation, 160 + variation);
                } else {
                    setpx(tx, ty, x, y, 160 + variation, 120 + variation, 60 + variation);
                }
            }
        }
    }

    // ================================================================
    // Upload
    // ================================================================
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void TextureAtlas::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void TextureAtlas::getUV(uint8_t texIdx, float& u0, float& v0, float& u1, float& v1) {
    // Atlas is laid out in 16-column rows (not 32) despite 512px / 16px = 32 fitting.
    // Tiles 0-15 in row 0, 16-31 in row 1, 32-47 in row 2, etc.
    constexpr int COLS = 16;
    int col = texIdx % COLS;
    int row = texIdx / COLS;
    constexpr float tileU = static_cast<float>(TILE_SIZE) / ATLAS_SIZE;
    constexpr float tileV = static_cast<float>(TILE_SIZE) / ATLAS_SIZE;
    u0 = col * tileU;  v0 = row * tileV;
    u1 = u0 + tileU;   v1 = v0 + tileV;
}

} // namespace voxelforge
