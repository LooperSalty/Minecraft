#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace voxelforge {

class InputManager {
public:
    static void init(GLFWwindow* window);
    static void update();

    static bool isKeyPressed(int key);
    static bool isKeyJustPressed(int key);
    static bool isMouseButtonPressed(int button);

    static glm::vec2 getMouseDelta();
    static glm::vec2 getMousePosition();
    static float getScrollDelta();

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static GLFWwindow* s_window;
    static glm::vec2 s_mousePos;
    static glm::vec2 s_lastMousePos;
    static glm::vec2 s_mouseDelta;
    static float s_scrollDelta;
    static bool s_firstMouse;
    static bool s_keys[GLFW_KEY_LAST + 1];
    static bool s_prevKeys[GLFW_KEY_LAST + 1];
};

} // namespace voxelforge
