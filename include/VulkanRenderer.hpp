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
    VkSwapchainKHR Swapchain;

    std::vector<SwapChainImage> swapChainImages;;

    // utilites
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;

    void createInstance();
    void createSurface();

    void getPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();

    bool checkExtensionSupport(const std::vector<const char*>& extensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkDeviceSuitable(VkPhysicalDevice device, int count);
    bool checkDeviceSurfaceSupport(); // Checks if logical device support surface

    QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& list);
    VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& list);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilites);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect);

public:
    VulkanRenderer();
    ~VulkanRenderer();

    int init(GLFWwindow* newWindow);
    void cleanup();
};
