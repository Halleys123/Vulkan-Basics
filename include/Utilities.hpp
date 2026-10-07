#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentationFamily = -1;

    bool isValid() {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilites;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentationModes;
};

struct SwapChainImage {
    VkImage image; // get
    VkImageView imageView; // created
};

const std::vector<const char*> deviceExtensions{"VK_KHR_swapchain"};
