#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>

namespace voxelforge {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    int getWidth() const  { return m_width; }
    int getHeight() const { return m_height; }
    float getAspectRatio() const;

    GLFWwindow* getHandle() const { return m_window; }
    void setCursorMode(int mode);

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    int m_width;
    int m_height;
};

} // namespace voxelforge
