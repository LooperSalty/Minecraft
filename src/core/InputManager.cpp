#include "InputManager.h"
#include <cstring>

namespace voxelforge {

GLFWwindow* InputManager::s_window      = nullptr;
glm::vec2   InputManager::s_mousePos    = {0.0f, 0.0f};
glm::vec2   InputManager::s_lastMousePos= {0.0f, 0.0f};
glm::vec2   InputManager::s_mouseDelta  = {0.0f, 0.0f};
float       InputManager::s_scrollDelta = 0.0f;
bool        InputManager::s_firstMouse  = true;
bool        InputManager::s_keys[GLFW_KEY_LAST + 1]     = {};
bool        InputManager::s_prevKeys[GLFW_KEY_LAST + 1]  = {};

void InputManager::init(GLFWwindow* window) {
    s_window = window;
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    std::memset(s_keys,     0, sizeof(s_keys));
    std::memset(s_prevKeys, 0, sizeof(s_prevKeys));
}

void InputManager::update() {
    std::memcpy(s_prevKeys, s_keys, sizeof(s_keys));
    s_scrollDelta = 0.0f;
}

bool InputManager::isKeyPressed(int key) {
    return (key >= 0 && key <= GLFW_KEY_LAST) && s_keys[key];
}

bool InputManager::isKeyJustPressed(int key) {
    return (key >= 0 && key <= GLFW_KEY_LAST) && s_keys[key] && !s_prevKeys[key];
}

bool InputManager::isMouseButtonPressed(int button) {
    return glfwGetMouseButton(s_window, button) == GLFW_PRESS;
}

glm::vec2 InputManager::getMouseDelta() {
    glm::vec2 d = s_mouseDelta;
    s_mouseDelta = {0.0f, 0.0f};
    return d;
}

glm::vec2 InputManager::getMousePosition() { return s_mousePos; }
float     InputManager::getScrollDelta()   { return s_scrollDelta; }

void InputManager::keyCallback(GLFWwindow*, int key, int, int action, int) {
    if (key < 0 || key > GLFW_KEY_LAST) return;
    if (action == GLFW_PRESS)        s_keys[key] = true;
    else if (action == GLFW_RELEASE) s_keys[key] = false;
}

void InputManager::mouseCallback(GLFWwindow*, double xpos, double ypos) {
    s_mousePos = {static_cast<float>(xpos), static_cast<float>(ypos)};
    if (s_firstMouse) { s_lastMousePos = s_mousePos; s_firstMouse = false; }
    s_mouseDelta.x += s_mousePos.x - s_lastMousePos.x;
    s_mouseDelta.y += s_lastMousePos.y - s_mousePos.y; // Y inverted
    s_lastMousePos = s_mousePos;
}

void InputManager::scrollCallback(GLFWwindow*, double, double yoffset) {
    s_scrollDelta = static_cast<float>(yoffset);
}

} // namespace voxelforge
