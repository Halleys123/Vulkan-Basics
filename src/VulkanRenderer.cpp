#include "VulkanRenderer.hpp"
#include "GLFW/glfw3.h"
#include "Utilities.hpp"

#include <set>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

VulkanRenderer::VulkanRenderer() {};
VulkanRenderer::~VulkanRenderer() {};

int VulkanRenderer::init(GLFWwindow* newWindow) {
    this->window = newWindow;
    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
    } catch (const std::runtime_error& err) {
        printf("Error: %s\n", err.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::cleanup() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::createInstance() {
    // Information about the application
    // In this structure only apiversion is important rest don't affect how app runs and are for debug information generation just in case
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0,0,1);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo vulkanInstanceInfo = {};
    vulkanInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vulkanInstanceInfo.pApplicationInfo = &appInfo;

    // GLFW extensions
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char*> instanceExtensions;
    for(size_t i = 0;i < glfwExtensionsCount;i += 1) {
        printf("GLFW Extension %llu::%s\n", i + 1, glfwExtensions[i]);
        instanceExtensions.push_back(glfwExtensions[i]);
    }

    if(checkExtensionSupport(instanceExtensions) == false) {
        throw std::runtime_error("VkInstance required extensions are not supported");
    }


    vulkanInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    vulkanInstanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

    vulkanInstanceInfo.enabledLayerCount = 0;
    vulkanInstanceInfo.ppEnabledLayerNames = nullptr;

    if(vkCreateInstance(&vulkanInstanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a runtime instance");
    }

    // return pInstance;
}
void VulkanRenderer::createSurface() {
    // This can be skipped if using glfw as glfw does this work cross platform for us.
    // Although if making application without GLFW then this part needs to managed manually so that compiler don't throw errors
    // VkWin32SurfaceCreateInfoKHR surfaceInfo;

    // vkCreateWin32SurfaceKHR(instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface)

    // Using glfw
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if(result != VK_SUCCESS) throw std::runtime_error("Failed to create a surface");

    // graphics queue contains presentation queue in it, so we rather check if any of the families support the presentation features
}

bool VulkanRenderer::checkExtensionSupport(const std::vector<const char*>& extensions) {
    // Standard query procedure in vulkan
    // When we query if vulkan support some extensions it returns total amount it supports and an array of items it supports.
    // but we can't pass it an array that it will fill when we initially don't know how many extensions it supports (because size of array is unknown)
    // So to solve this problem we first query vulkan to know how many extension it support
    // then create an array of required size
    // then again ask vulkan to fill that array with supported extensions
    //
    // but why vulkan can't just tell if an extension list we provided is supported by vulkan or even single extension at a time, are we just going to enumerate over the
    // items again becuase it just provided us the full list, that could be huge, wh;at re we doing herezz
    uint32_t totalExtensionSupport = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionSupport, nullptr);

    std::vector<VkExtensionProperties> supportedExtensions(totalExtensionSupport);
    vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionSupport, supportedExtensions.data());

    for(int i = 0;i < extensions.size();i += 1) {
        printf("Extension %d:: %s\n", i + 1, supportedExtensions[i].extensionName);
    }

    for(const auto &extension: extensions) {
        bool hasExtension = false;
        for(const VkExtensionProperties& supportedExtension : supportedExtensions) {
            if(strcmp(extension, supportedExtension.extensionName) == 0) {
                hasExtension = true;
                break;
            }
        }

        if(hasExtension == false) {
            printf("Extension: %s is not supported by VkInstance\n", extension);
            return false;
        }
    }
    return true;
}
bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t supportedExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedExtensionCount, nullptr);

    if(supportedExtensionCount == 0) return false;

    std::vector<VkExtensionProperties> extensions(supportedExtensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedExtensionCount, extensions.data());

    bool allSupported = true;
    for(const char* ext : deviceExtensions) {
        bool supported = false;
        for(auto& foundExt : extensions) {
            if(strcmp(foundExt.extensionName, ext) == 0) {
                supported = true;
                break;
            }
        }

        if(supported == false) {
            printf("%s Device Extension is not supported\n", ext);
            allSupported = false;

            // Not breaking here to get a list of all not supported extensions.
        }
    }

    if(allSupported == false) throw std::runtime_error("Device extensions are not supported");
    return true;
}
bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device, int count) {
    // There are multiple methods to check if device is suitable
    // Either using Device Name, ID or some other information about device iteself can be used for determining this
    // Second is to know what the device supports like geo shaders or other things can be usd to determine if device is good enough
    // VkPhysicalDeviceProperties properties;

    // printf("Device %d : %s [%d]\n", count + 1, properties.deviceName, properties.deviceType);
    // vkGetPhysicalDeviceProperties(device, &properties);

    // if(properties.deviceType == 2) {
    //     mainDevice.physicalDevice = device;
    //     return true;
    // }

    // Second method
    // VkPhysicalDeviceFeatures
    // vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures)

    // Queue Family - We need to make sure there are families that we need in our application
    // That is also done by checking what queues are supported by our device, so we get that information from our device

    QueueFamilyIndices inidices = getQueueFamilyIndices(device);
    bool extensionsSupport = checkDeviceExtensionSupport(device);

    bool swapchainValid = false;
    if(extensionsSupport) {
        SwapChainDetails details = getSwapChainDetails(device);
        swapchainValid = !details.presentationModes.empty() && !details.formats.empty();
    }

    return inidices.isValid() && extensionsSupport && swapchainValid;
}
bool VulkanRenderer::checkDeviceSurfaceSupport() {
    return true;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilyIndices(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t supportedFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &supportedFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> families(supportedFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &supportedFamilyCount, families.data());

  // Sometimes a card may support a family but may not have a single queue of that type so that is wrong
  // Sometime a single queue may support multiple type of QUEUE so that is why they used queueFlags so that &ing with both give non zero result
  for(int i = 0;i < supportedFamilyCount;i += 1) {
      if(families[i].queueCount >= 1 && families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          indices.graphicsFamily = i;
      }

      VkBool32 presentationSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);

      if(families[i].queueCount >= 1 && presentationSupport) indices.presentationFamily = i;

      if(indices.isValid()) {
          break;
      }
  }

  return indices;
}
SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
    SwapChainDetails swapchainDetails;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainDetails.surfaceCapabilites);

    uint32_t pSurfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &pSurfaceFormatCount, nullptr);
    if(pSurfaceFormatCount != 0) {
        swapchainDetails.formats.resize(pSurfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &pSurfaceFormatCount, swapchainDetails.formats.data());
    }

    uint32_t pPresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &pPresentModeCount, nullptr);
    if(pPresentModeCount != 0) {
        swapchainDetails.presentationModes.resize(pPresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &pPresentModeCount, swapchainDetails.presentationModes.data());
    }

    return swapchainDetails;
}

void VulkanRenderer::getPhysicalDevice() {
    uint32_t netPhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &netPhysicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDeviceList(netPhysicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &netPhysicalDeviceCount, physicalDeviceList.data());

    for(int i = 0;i < physicalDeviceList.size();i += 1) {
        if(checkDeviceSuitable(physicalDeviceList[i], i)) {
            mainDevice.physicalDevice = physicalDeviceList[i];
            break;
        }
    }

    if(physicalDeviceList.size() == 0) {
        throw std::runtime_error("No GPU on device supports Vulkan\n");
    }

    return;
}
void VulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = getQueueFamilyIndices(mainDevice.physicalDevice);

    float priority = 1.0f; // highgest 0.0 is lowest

    std::vector<VkDeviceQueueCreateInfo> queueFamiliesInfo;
    std::set<int> queueFamilyIndices = {indices.graphicsFamily, indices.presentationFamily};

    for(auto& index : queueFamilyIndices) {
        queueFamiliesInfo.push_back(VkDeviceQueueCreateInfo{});

        queueFamiliesInfo.back().sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueFamiliesInfo.back().queueFamilyIndex = index;
        queueFamiliesInfo.back().queueCount = 1;
        queueFamiliesInfo.back().pQueuePriorities = &priority;
    }

    VkPhysicalDeviceFeatures physicalDevicesFeatures = {};

    VkDeviceCreateInfo logicalDeviceInfo = {};
    logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceInfo.pEnabledFeatures = &physicalDevicesFeatures;
    logicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamiliesInfo.size());
    logicalDeviceInfo.pQueueCreateInfos = queueFamiliesInfo.data();
    logicalDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    logicalDeviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result = vkCreateDevice(mainDevice.physicalDevice, &logicalDeviceInfo, nullptr, &mainDevice.logicalDevice);

    if(result != VK_SUCCESS) throw std::runtime_error("Failed to create a logical device");

    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}
