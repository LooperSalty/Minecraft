#include "core/Window.h"
#include "core/InputManager.h"
#include "render/Shader.h"
#include "render/Camera.h"
#include "render/TextureAtlas.h"
#include "render/Frustum.h"
#include "world/ChunkManager.h"
#include "world/Chunk.h"
#include "world/Block.h"
#include "player/PlayerController.h"
#include "player/Inventory.h"
#include "gui/TextRenderer.h"
#include "gui/HudRenderer.h"
#include "entity/EntityManager.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <memory>
#include <vector>

using namespace voxelforge;

// --- Game states ---
enum class GameState { MainMenu, Playing, Paused, Dead, InventoryScreen };

// --- Configuration ---
static constexpr int64_t WORLD_SEED  = 12345;
static constexpr int     RENDER_DIST = 8;
static constexpr float   FOG_START   = static_cast<float>((RENDER_DIST - 2) * CHUNK_WIDTH);
static constexpr float   FOG_END     = static_cast<float>(RENDER_DIST * CHUNK_WIDTH);
static constexpr float   FOG_DENSITY = 1.0f / (RENDER_DIST * CHUNK_WIDTH);

// --- Day/night cycle constants ---
static constexpr float TICKS_PER_SECOND = 20.0f;
static constexpr float DAY_LENGTH       = 24000.0f;

// Sky color presets
static constexpr glm::vec3 SKY_DAY     = {0.53f, 0.81f, 0.92f};
static constexpr glm::vec3 SKY_SUNSET  = {0.90f, 0.50f, 0.30f};
static constexpr glm::vec3 SKY_NIGHT   = {0.01f, 0.01f, 0.05f};

// Spawn position
static constexpr glm::vec3 SPAWN_POS = {0.0f, 100.0f, 0.0f};

// --- Day/night helpers ---

static float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static glm::vec3 lerpColor(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

struct SkyState {
    glm::vec3 skyColor;
    glm::vec3 fogColor;
    float sunlight;
    glm::vec3 sunDirection;
};

static SkyState computeSkyState(float worldTime) {
    float t = worldTime / DAY_LENGTH; // 0..1

    glm::vec3 sky;
    float sunlight;

    if (t < 0.50f) {
        sky      = SKY_DAY;
        sunlight = 1.0f;
    } else if (t < 0.54f) {
        float f = smoothstep(0.50f, 0.54f, t);
        sky      = lerpColor(SKY_DAY, SKY_SUNSET, f);
        sunlight = 1.0f - 0.8f * f;
    } else if (t < 0.96f) {
        float entryFade = smoothstep(0.54f, 0.58f, t);
        sky      = lerpColor(SKY_SUNSET, SKY_NIGHT, entryFade);
        sunlight = 0.2f;
    } else {
        float f = smoothstep(0.96f, 1.00f, t);
        sky      = lerpColor(SKY_NIGHT, SKY_DAY, f);
        sunlight = 0.2f + 0.8f * f;
    }

    // Sun direction rotates over the day cycle
    float sunAngle = t * 2.0f * 3.14159265f;
    glm::vec3 sunDir = glm::normalize(glm::vec3(
        -std::cos(sunAngle), -std::sin(sunAngle), 0.3f));

    return { sky, sky, sunlight, sunDir };
}

// --- World resources managed via unique_ptr so we can create them on "Play" ---
struct WorldState {
    Camera         camera;
    PlayerController player;
    ChunkManager   chunkMgr;
    Frustum        frustum;
    EntityManager  entityMgr;
    float          worldTime   = 6000.0f;
    bool           creativeMode = false;
    bool           prevBreak    = false;
    bool           prevPlace    = false;
    float          attackCooldown = 0.0f;

    WorldState()
        : camera(SPAWN_POS)
        , player(SPAWN_POS)
        , chunkMgr(WORLD_SEED, RENDER_DIST)
    {}
};

int main() {
  try {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    std::fprintf(stderr, "VoxelForge: starting...\n");

    // --- Init ---
    Window window(854, 480, "VoxelForge v0.3");
    InputManager::init(window.getHandle());

    // --- Shaders (loaded once, shared across states) ---
    Shader blockShader;
    if (!blockShader.loadFromFiles("assets/shaders/block.vert",
                                   "assets/shaders/block.frag")) {
        std::fprintf(stderr, "Failed to load block shaders\n");
        return 1;
    }

    Shader lineShader;
    if (!lineShader.loadFromFiles("assets/shaders/line.vert",
                                  "assets/shaders/line.frag")) {
        std::fprintf(stderr, "Failed to load line shaders\n");
        return 1;
    }

    TextureAtlas atlas;
    atlas.generate();

    // --- GUI ---
    TextRenderer ui;
    ui.init(&lineShader);

    HudRenderer hud;
    hud.init(&ui);

    // --- Crosshair setup ---
    float crossSize = 0.02f;
    float crossVerts[] = {
        -crossSize, 0.0f, 0.0f,   crossSize, 0.0f, 0.0f,
        0.0f, -crossSize, 0.0f,   0.0f, crossSize, 0.0f
    };
    GLuint crossVAO, crossVBO;
    glGenVertexArrays(1, &crossVAO);
    glGenBuffers(1, &crossVBO);
    glBindVertexArray(crossVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crossVerts), crossVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- Block highlight wireframe setup ---
    float e = 0.002f;
    float cubeLines[] = {
        -e,-e,-e, 1+e,-e,-e,  1+e,-e,-e, 1+e,-e,1+e,
        1+e,-e,1+e, -e,-e,1+e,  -e,-e,1+e, -e,-e,-e,
        -e,1+e,-e, 1+e,1+e,-e,  1+e,1+e,-e, 1+e,1+e,1+e,
        1+e,1+e,1+e, -e,1+e,1+e,  -e,1+e,1+e, -e,1+e,-e,
        -e,-e,-e, -e,1+e,-e,  1+e,-e,-e, 1+e,1+e,-e,
        1+e,-e,1+e, 1+e,1+e,1+e,  -e,-e,1+e, -e,1+e,1+e,
    };
    GLuint hlVAO, hlVBO;
    glGenVertexArrays(1, &hlVAO);
    glGenBuffers(1, &hlVBO);
    glBindVertexArray(hlVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hlVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeLines), cubeLines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- OpenGL defaults ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // --- State ---
    GameState state = GameState::MainMenu;
    std::unique_ptr<WorldState> world;

    // Mouse-click rising-edge for UI
    bool prevMouseLeft = false;

    // Inventory screen state
    int inventoryFirstClick = -1;

    // FPS tracking
    double lastTime  = glfwGetTime();
    int    frames    = 0;
    double fpsTimer  = 0.0;
    int    lastFps   = 0;

    // Start with cursor visible (main menu)
    window.setCursorMode(GLFW_CURSOR_NORMAL);

    std::printf("VoxelForge v0.3 | seed=%lld | render_dist=%d\n",
                static_cast<long long>(WORLD_SEED), RENDER_DIST);

    // =========================================================================
    // Main loop
    // =========================================================================
    while (!window.shouldClose()) {
        double now = glfwGetTime();
        float  dt  = static_cast<float>(now - lastTime);
        lastTime   = now;

        InputManager::update();
        window.pollEvents();

        int w = window.getWidth();
        int h = window.getHeight();
        glm::vec2 mousePos  = InputManager::getMousePosition();
        bool curMouseLeft    = InputManager::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool mouseClick      = curMouseLeft && !prevMouseLeft;
        prevMouseLeft        = curMouseLeft;

        // =====================================================================
        switch (state) {
        // =====================================================================
        // MAIN MENU
        // =====================================================================
        case GameState::MainMenu: {
            glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            {
                float fw = static_cast<float>(w);
                float fh = static_cast<float>(h);
                glm::mat4 ortho = glm::ortho(0.0f, fw, fh, 0.0f);
                lineShader.use();
                lineShader.setMat4("uMVP", ortho);

                // Helper: draw filled rect
                auto drawRect = [&](float rx, float ry, float rw, float rh, const glm::vec4& col) {
                    lineShader.setVec4("uColor", col);
                    float v[] = {
                        rx,    ry,    0.0f,  rx+rw, ry,    0.0f,  rx+rw, ry+rh, 0.0f,
                        rx,    ry,    0.0f,  rx+rw, ry+rh, 0.0f,  rx,    ry+rh, 0.0f,
                    };
                    GLuint vao, vbo;
                    glGenVertexArrays(1, &vao);
                    glGenBuffers(1, &vbo);
                    glBindVertexArray(vao);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);
                    glEnableVertexAttribArray(0);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glBindVertexArray(0);
                    glDeleteBuffers(1, &vbo);
                    glDeleteVertexArrays(1, &vao);
                };

                // Helper: draw bitmap text using pixel quads (batched)
                // Uses the same 5x7 font data as TextRenderer
                static const uint8_t FONT[][7] = {
                    {0,0,0,0,0,0,0}, // 32 space
                    {4,4,4,4,4,0,4}, // 33 !
                    {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}, // 34-39
                    {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}, // 40-47
                    {14,17,19,21,25,17,14}, // 48 '0'
                    {4,12,4,4,4,4,14},       // 49 '1'
                    {14,17,1,6,8,16,31},     // 50 '2'
                    {14,17,1,6,1,17,14},     // 51 '3'
                    {2,6,10,18,31,2,2},      // 52 '4'
                    {31,16,30,1,1,17,14},    // 53 '5'
                    {6,8,16,30,17,17,14},    // 54 '6'
                    {31,1,2,4,8,8,8},        // 55 '7'
                    {14,17,17,14,17,17,14},  // 56 '8'
                    {14,17,17,15,1,2,12},    // 57 '9'
                    {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}, // 58-64
                    {14,17,17,31,17,17,17}, // 65 A
                    {30,17,17,30,17,17,30}, // B
                    {14,17,16,16,16,17,14}, // C
                    {28,18,17,17,17,18,28}, // D
                    {31,16,16,30,16,16,31}, // E
                    {31,16,16,30,16,16,16}, // F
                    {14,17,16,23,17,17,15}, // G
                    {17,17,17,31,17,17,17}, // H
                    {14,4,4,4,4,4,14},      // I
                    {7,2,2,2,2,18,12},      // J
                    {17,18,20,24,20,18,17}, // K
                    {16,16,16,16,16,16,31}, // L
                    {17,27,21,21,17,17,17}, // M
                    {17,25,21,19,17,17,17}, // N
                    {14,17,17,17,17,17,14}, // O
                    {30,17,17,30,16,16,16}, // P
                    {14,17,17,17,21,18,13}, // Q
                    {30,17,17,30,20,18,17}, // R
                    {14,17,16,14,1,17,14},  // S
                    {31,4,4,4,4,4,4},       // T
                    {17,17,17,17,17,17,14}, // U
                    {17,17,17,17,10,10,4},  // V
                    {17,17,17,21,21,27,17}, // W
                    {17,17,10,4,10,17,17},  // X
                    {17,17,10,4,4,4,4},     // Y
                    {31,1,2,4,8,16,31},     // Z  (90)
                };

                auto drawText = [&](const char* text, float tx, float ty, float scale, const glm::vec4& col) {
                    // Batch all pixel quads into one VBO
                    std::vector<float> verts;
                    float px = tx;
                    float ps = scale; // pixel size
                    for (const char* c = text; *c; ++c) {
                        int idx = -1;
                        char ch = *c;
                        if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
                        if (ch >= 32 && ch <= 90) idx = ch - 32;
                        if (idx < 0 || idx >= 59) { px += 6.0f * ps; continue; }

                        for (int row = 0; row < 7; ++row) {
                            uint8_t bits = FONT[idx][row];
                            for (int col = 0; col < 5; ++col) {
                                if ((bits >> (4 - col)) & 1) {
                                    float qx = px + col * ps;
                                    float qy = ty + row * ps;
                                    // Two triangles for one pixel
                                    verts.insert(verts.end(), {
                                        qx,    qy,    0.0f,  qx+ps, qy,    0.0f,  qx+ps, qy+ps, 0.0f,
                                        qx,    qy,    0.0f,  qx+ps, qy+ps, 0.0f,  qx,    qy+ps, 0.0f,
                                    });
                                }
                            }
                        }
                        px += 6.0f * ps;
                    }

                    if (verts.empty()) return;

                    lineShader.setVec4("uColor", col);
                    GLuint vao, vbo;
                    glGenVertexArrays(1, &vao);
                    glGenBuffers(1, &vbo);
                    glBindVertexArray(vao);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);
                    glEnableVertexAttribArray(0);
                    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / 3));
                    glBindVertexArray(0);
                    glDeleteBuffers(1, &vbo);
                    glDeleteVertexArrays(1, &vao);
                };

                // Measure text width helper
                auto textWidth = [](const char* text, float scale) -> float {
                    int len = 0;
                    for (const char* c = text; *c; ++c) len++;
                    return len > 0 ? (len * 6.0f - 1.0f) * scale : 0.0f;
                };

                // Centered text helper
                auto drawTextCentered = [&](const char* text, float cx, float cy, float scale, const glm::vec4& col) {
                    float tw = textWidth(text, scale);
                    drawText(text, cx - tw / 2.0f, cy, scale, col);
                };

                float cx = fw / 2.0f;
                float cy = fh / 2.0f;

                // Title
                drawTextCentered("VOXELFORGE", cx, cy - 120.0f, 6.0f,
                                 {1.0f, 1.0f, 1.0f, 1.0f});

                // Subtitle
                drawTextCentered("Minecraft 1.8.9 Clone", cx, cy - 65.0f, 2.0f,
                                 {0.6f, 0.6f, 0.6f, 1.0f});

                // Play button
                float btnW = 200.0f, btnH = 45.0f;
                float playX = cx - btnW/2.0f, playY = cy - 5.0f;
                bool hoverPlay = mousePos.x >= playX && mousePos.x <= playX+btnW &&
                                 mousePos.y >= playY && mousePos.y <= playY+btnH;
                drawRect(playX, playY, btnW, btnH,
                         hoverPlay ? glm::vec4(0.3f, 0.8f, 0.3f, 1.0f)
                                   : glm::vec4(0.2f, 0.6f, 0.2f, 1.0f));
                drawTextCentered("PLAY", cx, playY + 10.0f, 3.0f,
                                 {1.0f, 1.0f, 1.0f, 1.0f});

                // Quit button
                float quitY = playY + 60.0f;
                bool hoverQuit = mousePos.x >= playX && mousePos.x <= playX+btnW &&
                                 mousePos.y >= quitY && mousePos.y <= quitY+btnH;
                drawRect(playX, quitY, btnW, btnH,
                         hoverQuit ? glm::vec4(0.8f, 0.3f, 0.3f, 1.0f)
                                   : glm::vec4(0.6f, 0.2f, 0.2f, 1.0f));
                drawTextCentered("QUIT", cx, quitY + 10.0f, 3.0f,
                                 {1.0f, 1.0f, 1.0f, 1.0f});

                // Version
                drawText("v0.3", 10.0f, fh - 20.0f, 1.5f, {0.5f, 0.5f, 0.5f, 1.0f});

                // Click detection
                if (mouseClick && hoverPlay) {
                    world = std::make_unique<WorldState>();
                    world->entityMgr.init();
                    // Force-load chunks around spawn - try multiple positions to find dry land
                    auto getBlock = [&](int bx, int by, int bz) {
                        return world->chunkMgr.getBlock(bx, by, bz);
                    };
                    int spawnX = 0, spawnZ = 0;
                    int groundY = 80;
                    bool foundLand = false;
                    // Search in a spiral for dry land
                    for (int radius = 0; radius < 10 && !foundLand; ++radius) {
                        for (int dx = -radius; dx <= radius && !foundLand; ++dx) {
                            for (int dz = -radius; dz <= radius && !foundLand; ++dz) {
                                if (std::abs(dx) != radius && std::abs(dz) != radius) continue;
                                int tx = dx * 16, tz = dz * 16;
                                glm::vec3 testPos(static_cast<float>(tx), 100.0f, static_cast<float>(tz));
                                world->chunkMgr.update(testPos);
                                // Scan from top to find solid ground (not water)
                                for (int y = 200; y >= 0; --y) {
                                    BlockType bt = getBlock(tx, y, tz);
                                    if (isBlockSolid(bt)) {
                                        // Check block above is not water
                                        BlockType above = getBlock(tx, y + 1, tz);
                                        if (above == BlockType::Air) {
                                            spawnX = tx;
                                            spawnZ = tz;
                                            groundY = y + 1;
                                            foundLand = true;
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    glm::vec3 safeSpawn(static_cast<float>(spawnX) + 0.5f,
                                        static_cast<float>(groundY),
                                        static_cast<float>(spawnZ) + 0.5f);
                    world->player.setPosition(safeSpawn);
                    world->camera.setPosition(world->player.getEyePosition());
                    world->creativeMode = false;
                    state = GameState::Playing;
                    window.setCursorMode(GLFW_CURSOR_DISABLED);
                    InputManager::resetFirstMouse();
                }
                if (mouseClick && hoverQuit) {
                    glfwSetWindowShouldClose(window.getHandle(), true);
                }
            }

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            break;
        }

        // =====================================================================
        // PLAYING
        // =====================================================================
        case GameState::Playing: {
            // --- Escape -> Paused ---
            if (InputManager::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
                std::fprintf(stderr, "ESC pressed -> Paused\n");
                state = GameState::Paused;
                window.setCursorMode(GLFW_CURSOR_NORMAL);
                break;
            }

            // --- E -> Inventory screen ---
            if (InputManager::isKeyJustPressed(GLFW_KEY_E)) {
                state = GameState::InventoryScreen;
                inventoryFirstClick = -1;
                window.setCursorMode(GLFW_CURSOR_NORMAL);
                break;
            }

            // --- Number keys 1-9 -> select hotbar slot ---
            for (int k = 0; k < 9; ++k) {
                if (InputManager::isKeyJustPressed(GLFW_KEY_1 + k)) {
                    world->player.setSelectedSlot(k);
                }
            }

            // F1: wireframe toggle
            if (InputManager::isKeyJustPressed(GLFW_KEY_F1)) {
                static bool wire = false;
                wire = !wire;
                glPolygonMode(GL_FRONT_AND_BACK, wire ? GL_LINE : GL_FILL);
            }

            // F5: creative/survival toggle
            if (InputManager::isKeyJustPressed(GLFW_KEY_F5)) {
                world->creativeMode = !world->creativeMode;
                std::printf("Mode: %s\n", world->creativeMode ? "Creative" : "Survival");
            }

            // F11: fullscreen toggle
            if (InputManager::isKeyJustPressed(GLFW_KEY_F11)) {
                static bool fullscreen = false;
                fullscreen = !fullscreen;
                if (fullscreen) {
                    GLFWmonitor* mon = glfwGetPrimaryMonitor();
                    const GLFWvidmode* mode = glfwGetVideoMode(mon);
                    glfwSetWindowMonitor(window.getHandle(), mon,
                        0, 0, mode->width, mode->height, mode->refreshRate);
                } else {
                    glfwSetWindowMonitor(window.getHandle(), nullptr,
                        100, 100, 854, 480, 0);
                }
            }

            // --- Build InputState ---
            PlayerController::InputState input;
            input.forward    = InputManager::isKeyPressed(GLFW_KEY_Z) || InputManager::isKeyPressed(GLFW_KEY_W);
            input.backward   = InputManager::isKeyPressed(GLFW_KEY_S);
            input.left       = InputManager::isKeyPressed(GLFW_KEY_Q) || InputManager::isKeyPressed(GLFW_KEY_A);
            input.right      = InputManager::isKeyPressed(GLFW_KEY_D);
            input.jump       = InputManager::isKeyPressed(GLFW_KEY_SPACE);
            input.sprint     = InputManager::isKeyPressed(GLFW_KEY_LEFT_CONTROL);
            input.sneak      = InputManager::isKeyPressed(GLFW_KEY_LEFT_SHIFT);
            input.breakBlock = InputManager::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
            input.placeBlock = InputManager::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
            input.scrollDelta = InputManager::getScrollDelta();
            input.mouseDelta  = InputManager::getMouseDelta();
            input.creative    = world->creativeMode;

            // --- Block accessor lambda ---
            auto blockAccessor = [&](int bx, int by, int bz) {
                return world->chunkMgr.getBlock(bx, by, bz);
            };

            // --- Player update ---
            world->player.update(dt, input, blockAccessor);

            // --- Camera follows player ---
            world->camera.setPosition(world->player.getEyePosition());
            world->camera.setYawPitch(world->player.getYaw(), world->player.getPitch());

            // --- Decrement attack cooldown ---
            world->attackCooldown -= dt;
            if (world->attackCooldown < 0.0f) world->attackCooldown = 0.0f;

            // --- Block break/place on rising edge ---
            bool curBreak = input.breakBlock;
            bool curPlace = input.placeBlock;

            if (curBreak && !world->prevBreak) {
                // Try entity raycast first (combat)
                bool hitEntity = false;
                if (world->attackCooldown <= 0.0f) {
                    glm::vec3 eyePos = world->player.getEyePosition();
                    glm::vec3 front  = world->player.getFront();
                    auto eHit = world->entityMgr.raycastEntity(eyePos, front, 5.0f);
                    if (eHit.hit) {
                        hitEntity = true;
                        // Knockback direction: from player toward entity (horizontal)
                        glm::vec3 knockDir = front;
                        knockDir.y = 0.0f;
                        float len = std::sqrt(knockDir.x * knockDir.x + knockDir.z * knockDir.z);
                        if (len > 0.01f) {
                            knockDir.x /= len;
                            knockDir.z /= len;
                        }
                        world->entityMgr.damageEntity(eHit.entityIndex, 4.0f, knockDir);
                        world->attackCooldown = 0.5f;
                    }
                }
                // If no entity hit, proceed with block breaking
                if (!hitEntity) {
                    world->player.breakBlock(
                        [&](int bx, int by, int bz, BlockType bt) { world->chunkMgr.setBlock(bx, by, bz, bt); },
                        blockAccessor);
                }
            }
            if (curPlace && !world->prevPlace) {
                world->player.placeBlock(
                    [&](int bx, int by, int bz, BlockType bt) { world->chunkMgr.setBlock(bx, by, bz, bt); },
                    blockAccessor);
            }

            world->prevBreak = curBreak;
            world->prevPlace = curPlace;

            // --- Day/night cycle ---
            world->worldTime += dt * TICKS_PER_SECOND;
            if (world->worldTime >= DAY_LENGTH) {
                world->worldTime -= DAY_LENGTH;
            }

            SkyState sky = computeSkyState(world->worldTime);
            float dayProgress = world->worldTime / DAY_LENGTH;

            // --- Entity update ---
            world->entityMgr.update(dt, world->player.getPosition(), blockAccessor, dayProgress);

            // --- Mob damage (survival only) ---
            if (!world->creativeMode) {
                float mobDmg = world->entityMgr.checkPlayerDamage(world->player.getPosition());
                if (mobDmg > 0.0f) {
                    world->player.takeDamage(mobDmg);
                }
            }

            // --- Check death ---
            if (world->player.getHealth() <= 0.0f && !world->creativeMode) {
                state = GameState::Dead;
                window.setCursorMode(GLFW_CURSOR_NORMAL);
                break;
            }

            // --- World update ---
            world->chunkMgr.update(world->player.getPosition());

            // --- Render world ---
            glClearColor(sky.skyColor.r, sky.skyColor.g, sky.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = world->camera.getProjectionMatrix(window.getAspectRatio())
                          * world->camera.getViewMatrix();

            world->frustum.update(vp);

            blockShader.use();
            blockShader.setMat4("uViewProjection", vp);
            blockShader.setVec3("uCameraPos", world->camera.getPosition());
            blockShader.setFloat("uFogStart", FOG_START);
            blockShader.setFloat("uFogEnd",   FOG_END);
            blockShader.setFloat("uFogDensity", FOG_DENSITY);
            blockShader.setVec3("uFogColor",  sky.fogColor);
            blockShader.setVec3("uSunDirection", sky.sunDirection);
            blockShader.setFloat("uTime", static_cast<float>(now));
            blockShader.setInt("uTextureAtlas", 0);

            atlas.bind(0);
            world->chunkMgr.renderAll(blockShader, world->frustum);

            // --- Transparent pass (water, glass, ice) ---
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            world->chunkMgr.renderTransparent(blockShader, world->frustum);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);

            // --- Entities ---
            world->entityMgr.render(vp);

            // --- Block highlight ---
            auto hit = world->player.raycast(blockAccessor);
            if (hit.hit) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(hit.blockPos));
                lineShader.use();
                lineShader.setMat4("uMVP", vp * model);
                lineShader.setVec4("uColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                glLineWidth(2.0f);
                glBindVertexArray(hlVAO);
                glDrawArrays(GL_LINES, 0, 24);
                glBindVertexArray(0);
            }

            // --- Crosshair ---
            glDisable(GL_DEPTH_TEST);
            lineShader.use();
            lineShader.setMat4("uMVP", glm::mat4(1.0f));
            lineShader.setVec4("uColor", glm::vec4(1.0f));
            glLineWidth(2.0f);
            glBindVertexArray(crossVAO);
            glDrawArrays(GL_LINES, 0, 4);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);

            // --- HUD overlay ---
            glDisable(GL_DEPTH_TEST);
            hud.drawPlayingHUD(world->player, lastFps,
                               world->chunkMgr.getLoadedCount(),
                               world->entityMgr.getCount(),
                               world->creativeMode, w, h);
            glEnable(GL_DEPTH_TEST);

            // --- FPS counter & title bar ---
            ++frames;
            fpsTimer += dt;
            if (fpsTimer >= 1.0) {
                lastFps  = frames;
                frames   = 0;
                fpsTimer = 0.0;

                char title[256];
                const auto& pos = world->player.getPosition();
                std::snprintf(title, sizeof(title),
                    "VoxelForge | %d FPS | pos(%.1f,%.1f,%.1f) | HP:%.0f | [%s] | %s | chunks:%d | mobs:%d",
                    lastFps, pos.x, pos.y, pos.z,
                    world->player.getHealth(),
                    getBlockData(world->player.getSelectedBlock()).name,
                    world->creativeMode ? "Creative" : "Survival",
                    world->chunkMgr.getLoadedCount(),
                    world->entityMgr.getCount());
                glfwSetWindowTitle(window.getHandle(), title);
            }

            break;
        }

        // =====================================================================
        // PAUSED
        // =====================================================================
        case GameState::Paused: {
            // Esc -> Resume
            if (InputManager::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
                state = GameState::Playing;
                window.setCursorMode(GLFW_CURSOR_DISABLED);
                InputManager::resetFirstMouse();
                break;
            }

            // --- Render frozen world behind ---
            SkyState sky = computeSkyState(world->worldTime);
            glClearColor(sky.skyColor.r, sky.skyColor.g, sky.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = world->camera.getProjectionMatrix(window.getAspectRatio())
                          * world->camera.getViewMatrix();

            world->frustum.update(vp);

            blockShader.use();
            blockShader.setMat4("uViewProjection", vp);
            blockShader.setVec3("uCameraPos", world->camera.getPosition());
            blockShader.setFloat("uFogStart", FOG_START);
            blockShader.setFloat("uFogEnd",   FOG_END);
            blockShader.setFloat("uFogDensity", FOG_DENSITY);
            blockShader.setVec3("uFogColor",  sky.fogColor);
            blockShader.setVec3("uSunDirection", sky.sunDirection);
            blockShader.setFloat("uTime", static_cast<float>(glfwGetTime()));
            blockShader.setInt("uTextureAtlas", 0);

            atlas.bind(0);
            world->chunkMgr.renderAll(blockShader, world->frustum);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            world->chunkMgr.renderTransparent(blockShader, world->frustum);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            world->entityMgr.render(vp);

            // --- Dark overlay ---
            glDisable(GL_DEPTH_TEST);
            ui.drawRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h),
                        {0.0f, 0.0f, 0.0f, 0.5f}, w, h);

            float cx = static_cast<float>(w) / 2.0f;
            float cy = static_cast<float>(h) / 2.0f;

            // Title
            ui.drawTextCentered("PAUSED", cx, cy - 60.0f, 4.0f,
                                {1.0f, 1.0f, 1.0f, 1.0f}, w, h);

            // Resume button
            if (ui.drawButton("Resume", cx, cy + 10.0f, 60.0f, 12.0f, 2.5f,
                              {0.2f, 0.6f, 0.2f, 0.9f}, {1.0f, 1.0f, 1.0f, 1.0f},
                              w, h, mousePos, mouseClick)) {
                state = GameState::Playing;
                window.setCursorMode(GLFW_CURSOR_DISABLED);
                InputManager::resetFirstMouse();
            }

            // Quit button
            if (ui.drawButton("Quit", cx, cy + 70.0f, 60.0f, 12.0f, 2.5f,
                              {0.6f, 0.2f, 0.2f, 0.9f}, {1.0f, 1.0f, 1.0f, 1.0f},
                              w, h, mousePos, mouseClick)) {
                glfwSetWindowShouldClose(window.getHandle(), true);
            }

            glEnable(GL_DEPTH_TEST);
            break;
        }

        // =====================================================================
        // DEAD
        // =====================================================================
        case GameState::Dead: {
            // --- Render frozen world behind ---
            SkyState sky = computeSkyState(world->worldTime);
            glClearColor(sky.skyColor.r, sky.skyColor.g, sky.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = world->camera.getProjectionMatrix(window.getAspectRatio())
                          * world->camera.getViewMatrix();

            world->frustum.update(vp);

            blockShader.use();
            blockShader.setMat4("uViewProjection", vp);
            blockShader.setVec3("uCameraPos", world->camera.getPosition());
            blockShader.setFloat("uFogStart", FOG_START);
            blockShader.setFloat("uFogEnd",   FOG_END);
            blockShader.setFloat("uFogDensity", FOG_DENSITY);
            blockShader.setVec3("uFogColor",  sky.fogColor);
            blockShader.setVec3("uSunDirection", sky.sunDirection);
            blockShader.setFloat("uTime", static_cast<float>(glfwGetTime()));
            blockShader.setInt("uTextureAtlas", 0);

            atlas.bind(0);
            world->chunkMgr.renderAll(blockShader, world->frustum);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            world->chunkMgr.renderTransparent(blockShader, world->frustum);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            world->entityMgr.render(vp);

            // --- Red-tinted overlay ---
            glDisable(GL_DEPTH_TEST);
            ui.drawRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h),
                        {0.6f, 0.0f, 0.0f, 0.4f}, w, h);

            float cx = static_cast<float>(w) / 2.0f;
            float cy = static_cast<float>(h) / 2.0f;

            // Title
            ui.drawTextCentered("You Died!", cx, cy - 60.0f, 4.0f,
                                {1.0f, 0.2f, 0.2f, 1.0f}, w, h);

            // Respawn button
            if (ui.drawButton("Respawn", cx, cy + 10.0f, 60.0f, 12.0f, 2.5f,
                              {0.2f, 0.6f, 0.2f, 0.9f}, {1.0f, 1.0f, 1.0f, 1.0f},
                              w, h, mousePos, mouseClick)) {
                world->player.resetHealth();
                world->player.setPosition(SPAWN_POS);
                state = GameState::Playing;
                window.setCursorMode(GLFW_CURSOR_DISABLED);
                InputManager::resetFirstMouse();
            }

            glEnable(GL_DEPTH_TEST);
            break;
        }

        // =====================================================================
        // INVENTORY SCREEN
        // =====================================================================
        case GameState::InventoryScreen: {
            // E or Escape -> back to playing
            if (InputManager::isKeyJustPressed(GLFW_KEY_E) ||
                InputManager::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
                state = GameState::Playing;
                inventoryFirstClick = -1;
                window.setCursorMode(GLFW_CURSOR_DISABLED);
                InputManager::resetFirstMouse();
                break;
            }

            // --- Render frozen world behind ---
            SkyState sky = computeSkyState(world->worldTime);
            glClearColor(sky.skyColor.r, sky.skyColor.g, sky.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = world->camera.getProjectionMatrix(window.getAspectRatio())
                          * world->camera.getViewMatrix();

            world->frustum.update(vp);

            blockShader.use();
            blockShader.setMat4("uViewProjection", vp);
            blockShader.setVec3("uCameraPos", world->camera.getPosition());
            blockShader.setFloat("uFogStart", FOG_START);
            blockShader.setFloat("uFogEnd",   FOG_END);
            blockShader.setFloat("uFogDensity", FOG_DENSITY);
            blockShader.setVec3("uFogColor",  sky.fogColor);
            blockShader.setVec3("uSunDirection", sky.sunDirection);
            blockShader.setFloat("uTime", static_cast<float>(glfwGetTime()));
            blockShader.setInt("uTextureAtlas", 0);

            atlas.bind(0);
            world->chunkMgr.renderAll(blockShader, world->frustum);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            world->chunkMgr.renderTransparent(blockShader, world->frustum);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            world->entityMgr.render(vp);

            // --- Inventory overlay ---
            glDisable(GL_DEPTH_TEST);

            int clickedSlot = -1;
            hud.drawInventoryScreen(world->player.getInventory(), w, h,
                                    mousePos, mouseClick, clickedSlot);

            // Handle slot clicking for swap
            if (clickedSlot >= 0) {
                if (inventoryFirstClick < 0) {
                    inventoryFirstClick = clickedSlot;
                } else {
                    world->player.getInventory().swapSlots(inventoryFirstClick, clickedSlot);
                    inventoryFirstClick = -1;
                }
            }

            // Show selected slot indicator
            if (inventoryFirstClick >= 0) {
                const auto& sel = world->player.getInventory().getSlot(inventoryFirstClick);
                if (!sel.isEmpty()) {
                    const char* name = getBlockData(sel.type).name;
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "Moving: %s x%d", name, sel.count);
                    ui.drawTextCentered(buf, static_cast<float>(w) / 2.0f,
                                        static_cast<float>(h) - 20.0f, 2.0f,
                                        {1.0f, 1.0f, 0.5f, 1.0f}, w, h);
                }
            }

            glEnable(GL_DEPTH_TEST);
            break;
        }
        } // switch

        window.swapBuffers();
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1, &crossVAO);
    glDeleteBuffers(1, &crossVBO);
    glDeleteVertexArrays(1, &hlVAO);
    glDeleteBuffers(1, &hlVBO);

    ui.cleanup();
    if (world) {
        world->entityMgr.cleanup();
    }

    return 0;
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "FATAL: %s\n", ex.what());
    return 1;
  } catch (...) {
    std::fprintf(stderr, "FATAL: unknown exception\n");
    return 1;
  }
}
