#include <cstdlib>
#include <stdio.h>
#include <string>

#include <vulkan/vulkan.h>
#include "VulkanRenderer.hpp"

#include "GLFW/glfw3.h"
// #include "glm/glm.hpp"
// #include "glm/mat4x4.hpp"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

int initWindow(std::string wName =  "Vulkan Window", const int height = 1280, const int width = 720) {
    glfwInit();
    // By default GLFW works with OpenGL rather than Vulkan, so here we are telling it needs to configure for opengl
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // because we need to recreate object is window is resized

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    window = glfwCreateWindow(mode->width / 2, (mode->width * 10) / (16 * 2) , wName.c_str(), nullptr, nullptr);
    if(vulkanRenderer.init(window) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    vulkanRenderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int main() {
    #ifdef DEBUG_MODE
        printf("Running in Debug Mode\n");
    #endif

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    printf("Total extensions: %i\n", extension_count);

    initWindow("Vulkan learning", 1280, 720);

    return 0;
}
