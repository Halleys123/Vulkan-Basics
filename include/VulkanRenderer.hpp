#pragma once

#include <GLFW/glfw3.h>
#include "Utilities.hpp"
#include <vector>

class VulkanRenderer {
private:
    GLFWwindow* window;
    VkInstance instance;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;

    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;

    void createInstance();
    void createSurface();

    void getPhysicalDevice();
    void createLogicalDevice();

    bool checkExtensionSupport(const std::vector<const char*>& extensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkDeviceSuitable(VkPhysicalDevice device, int count);
    bool checkDeviceSurfaceSupport(); // Checks if logical device support surface

    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
public:
    VulkanRenderer();
    ~VulkanRenderer();

    int init(GLFWwindow* newWindow);
    void cleanup();
};
