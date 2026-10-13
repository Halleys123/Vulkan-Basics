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

    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    std::vector<SwapChainImage> swapChainImages;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    std::vector<VkCommandBuffer> commandBuffer;

    // utilites
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    VkCommandPool graphicsCommandPool; // This is a command pool only used for graphics

    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;

    void createInstance();
    void createSurface();

    void getPhysicalDevice();
    void recordCommands();

    void createLogicalDevice();
    void createSwapChain();
    void createGraphicsPipeline();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();

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
    VkShaderModule createShaderModule(const FileContent& shaderBinary);

public:
    VulkanRenderer();
    ~VulkanRenderer();

    int init(GLFWwindow* newWindow);
    void cleanup();
};
