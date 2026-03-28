#include "core/Window.h"
#include "core/InputManager.h"
#include "render/Shader.h"
#include "render/Camera.h"
#include "render/TextureAtlas.h"
#include "render/Frustum.h"
#include "world/ChunkManager.h"
#include "world/Chunk.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <cmath>
#include <algorithm>

using namespace voxelforge;

// --- Configuration ---
static constexpr int64_t WORLD_SEED  = 12345;
static constexpr int     RENDER_DIST = 8;
static constexpr float   FOG_START   = static_cast<float>((RENDER_DIST - 2) * CHUNK_WIDTH);
static constexpr float   FOG_END     = static_cast<float>(RENDER_DIST * CHUNK_WIDTH);

// --- Day/night cycle constants ---
static constexpr float TICKS_PER_SECOND = 20.0f;
static constexpr float DAY_LENGTH       = 24000.0f;

// Sky color presets
static constexpr glm::vec3 SKY_DAY     = {0.53f, 0.81f, 0.92f};
static constexpr glm::vec3 SKY_SUNSET  = {0.90f, 0.50f, 0.30f};
static constexpr glm::vec3 SKY_NIGHT   = {0.01f, 0.01f, 0.05f};

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
};

static SkyState computeSkyState(float worldTime) {
    float t = worldTime / DAY_LENGTH; // 0..1

    glm::vec3 sky;
    float sunlight;

    if (t < 0.50f) {
        // Day
        sky      = SKY_DAY;
        sunlight = 1.0f;
    } else if (t < 0.54f) {
        // Sunset (day -> sunset -> night)
        float f = smoothstep(0.50f, 0.54f, t);
        sky      = lerpColor(SKY_DAY, SKY_SUNSET, f);
        sunlight = 1.0f - 0.8f * f;
    } else if (t < 0.96f) {
        // Night
        // Smooth transition from sunset color to full night at ~0.58
        float entryFade = smoothstep(0.54f, 0.58f, t);
        sky      = lerpColor(SKY_SUNSET, SKY_NIGHT, entryFade);
        sunlight = 0.2f;
    } else {
        // Sunrise (night -> day)
        float f = smoothstep(0.96f, 1.00f, t);
        sky      = lerpColor(SKY_NIGHT, SKY_DAY, f);
        sunlight = 0.2f + 0.8f * f;
    }

    return { sky, sky, sunlight };
}

int main() {
    // --- Init ---
    Window window(854, 480, "VoxelForge v0.1");
    window.setCursorMode(GLFW_CURSOR_DISABLED);
    InputManager::init(window.getHandle());

    Shader blockShader;
    if (!blockShader.loadFromFiles("assets/shaders/block.vert",
                                   "assets/shaders/block.frag")) {
        std::fprintf(stderr, "Failed to load shaders\n");
        return 1;
    }

    TextureAtlas atlas;
    atlas.generate();

    Camera camera(glm::vec3(0.0f, 80.0f, 0.0f));

    ChunkManager chunkMgr(WORLD_SEED, RENDER_DIST);
    Frustum frustum;

    // Start at morning (tick 6000 ~ 6:00 AM)
    float worldTime = 6000.0f;

    // --- OpenGL state ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    double lastTime = glfwGetTime();
    int    frames   = 0;
    double fpsTimer = 0.0;
    int    lastFps  = 0;

    std::printf("VoxelForge Phase 1 | seed=%lld | render_dist=%d\n",
                static_cast<long long>(WORLD_SEED), RENDER_DIST);
    std::printf("ZQSD/WASD=move  Mouse=look  Space/Shift=up/down  Ctrl=sprint\n");
    std::printf("F1=wireframe  F11=fullscreen  Esc=quit\n");

    // --- Main loop ---
    while (!window.shouldClose()) {
        double now = glfwGetTime();
        float  dt  = static_cast<float>(now - lastTime);
        lastTime   = now;

        // FPS counter
        ++frames;
        fpsTimer += dt;
        if (fpsTimer >= 1.0) {
            lastFps  = frames;
            frames   = 0;
            fpsTimer = 0.0;

            char title[256];
            const auto& pos = camera.getPosition();
            std::snprintf(title, sizeof(title),
                "VoxelForge v0.1 | %d FPS | pos(%.1f, %.1f, %.1f) | chunks: %d | time: %.0f",
                lastFps, pos.x, pos.y, pos.z,
                chunkMgr.getLoadedCount(),
                worldTime);
            glfwSetWindowTitle(window.getHandle(), title);
        }

        // --- Input ---
        window.pollEvents();
        InputManager::update();

        if (InputManager::isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getHandle(), true);
        }

        // F1: wireframe toggle
        if (InputManager::isKeyJustPressed(GLFW_KEY_F1)) {
            static bool wire = false;
            wire = !wire;
            glPolygonMode(GL_FRONT_AND_BACK, wire ? GL_LINE : GL_FILL);
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

        // Camera movement: AZERTY (ZQSD) + QWERTY (WASD)
        glm::vec2 md = InputManager::getMouseDelta();
        bool fwd   = InputManager::isKeyPressed(GLFW_KEY_Z) || InputManager::isKeyPressed(GLFW_KEY_W);
        bool back  = InputManager::isKeyPressed(GLFW_KEY_S);
        bool left  = InputManager::isKeyPressed(GLFW_KEY_Q) || InputManager::isKeyPressed(GLFW_KEY_A);
        bool right = InputManager::isKeyPressed(GLFW_KEY_D);
        camera.update(dt, fwd, back, left, right,
            InputManager::isKeyPressed(GLFW_KEY_SPACE),
            InputManager::isKeyPressed(GLFW_KEY_LEFT_SHIFT),
            InputManager::isKeyPressed(GLFW_KEY_LEFT_CONTROL),
            md);

        // --- Day/night cycle ---
        worldTime += dt * TICKS_PER_SECOND;
        if (worldTime >= DAY_LENGTH) {
            worldTime -= DAY_LENGTH;
        }

        SkyState sky = computeSkyState(worldTime);

        // --- World update ---
        chunkMgr.update(camera.getPosition());

        // --- Render ---
        glClearColor(sky.skyColor.r, sky.skyColor.g, sky.skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 vp = camera.getProjectionMatrix(window.getAspectRatio())
                      * camera.getViewMatrix();

        frustum.update(vp);

        blockShader.use();
        blockShader.setMat4("uViewProjection", vp);
        blockShader.setVec3("uCameraPos", camera.getPosition());
        blockShader.setFloat("uFogStart", FOG_START);
        blockShader.setFloat("uFogEnd",   FOG_END);
        blockShader.setVec3("uFogColor",  sky.fogColor);
        blockShader.setInt("uTextureAtlas", 0);

        atlas.bind(0);

        chunkMgr.renderAll(blockShader, frustum);

        window.swapBuffers();
    }

    return 0;
}
