#include "VulkanRenderer.hpp"
#include "GLFW/glfw3.h"
#include "Utilities.hpp"

#include <limits>
#include <set>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

#include <algorithm>

VulkanRenderer::VulkanRenderer() {};
VulkanRenderer::~VulkanRenderer() {};

int VulkanRenderer::init(GLFWwindow* newWindow) {
    this->window = newWindow;
    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createGraphicsPipeline();
    } catch (const std::runtime_error& err) {
        printf("Error: %s\n", err.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::cleanup() {
    for(SwapChainImage& i : swapChainImages) {
        vkDestroyImageView(mainDevice.logicalDevice, i.imageView, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroySwapchainKHR(mainDevice.logicalDevice, Swapchain, nullptr);
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

    // // Image Counts
    // printf("Min Image Count: %u\n", swapchainDetails.surfaceCapabilites.minImageCount);
    // printf("Max Image Count: %u\n", swapchainDetails.surfaceCapabilites.maxImageCount);

    // // Extents (Resolutions)
    // printf("Current Extent:  %u x %u\n", swapchainDetails.surfaceCapabilites.currentExtent.width,
    //                                      swapchainDetails.surfaceCapabilites.currentExtent.height);
    // printf("Min Image Extent: %u x %u\n", swapchainDetails.surfaceCapabilites.minImageExtent.width,
    //                                      swapchainDetails.surfaceCapabilites.minImageExtent.height);
    // printf("Max Image Extent: %u x %u\n", swapchainDetails.surfaceCapabilites.maxImageExtent.width,
    //                                      swapchainDetails.surfaceCapabilites.maxImageExtent.height);

    // // Array Layers (From your example)
    // printf("Max Image Layers: %u\n", swapchainDetails.surfaceCapabilites.maxImageArrayLayers);

    // // Transforms & Orientation (Flags & Enums)
    // printf("Supported Transforms:   0x%X\n", swapchainDetails.surfaceCapabilites.supportedTransforms);
    // printf("Current Transform:     0x%X\n", swapchainDetails.surfaceCapabilites.currentTransform);

    // // Alpha Compositing Flags
    // printf("Supported Composite Alpha: 0x%X\n", swapchainDetails.surfaceCapabilites.supportedCompositeAlpha);

    // // Image Usage Capabilities
    // printf("Supported Usage Flags:     0x%X\n", swapchainDetails.surfaceCapabilites.supportedUsageFlags);


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
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    //Best format is subjective, but
    // here I will use VK_FORMAT_R8G8B8A8_UNORM
    // colorSPace will be VK_COLOR_SPACE_SRGB_NONLINEAR_KHR

    if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        // This is Vulkans way of saying that all formats are available
        return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for(const auto& i : formats) {
        if(i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && (i.format == VK_FORMAT_R8G8B8A8_UNORM || i.format == VK_FORMAT_B8G8R8A8_UNORM)) {
            return i;
        }
    }

    printf("No Surface format available.\n");

    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> &list) {
    // Optimal is Mailbox so we will try to find that otherwise we will work with other
    bool secondBestAvail = false;
    for(const auto& i : list) {
        if(i == VK_PRESENT_MODE_MAILBOX_KHR) return i;
        if(i == VK_PRESENT_MODE_FIFO_KHR) secondBestAvail = true;
    }

    if(secondBestAvail == false) {
        // This can only happen if driver are buggy or something liek that otherwise these are always available
        throw std::runtime_error("Either your drivers are outdated or your GPU is faulty, No Presentation mode found");
    }

    // Vulkan spec says it must be always available.
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilites) {
    if(surfaceCapabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surfaceCapabilites.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D newExtent{};
    newExtent.width = static_cast<uint32_t>(width);
    newExtent.height = static_cast<uint32_t>(height);

    newExtent.width = std::clamp(newExtent.width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width);
    newExtent.height = std::clamp(newExtent.height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height);
    return newExtent;
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
        queueFamiliesInfo.back().pQueuePriorities = &priority;
    }
        queueFamiliesInfo.back().queueCount = 1;

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
void VulkanRenderer::createSwapChain() {
    SwapChainDetails details = getSwapChainDetails(mainDevice.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = chooseBestPresentationMode(details.presentationModes);
    VkExtent2D extent = chooseSwapExtent(details.surfaceCapabilites);

    // -------------------------

    uint32_t imageCount = details.surfaceCapabilites.minImageCount + 1;
    if(details.surfaceCapabilites.maxImageCount > 0 && imageCount > details.surfaceCapabilites.maxImageCount) {
        imageCount = details.surfaceCapabilites.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1; // Number of layers for our image in chain
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.preTransform = details.surfaceCapabilites.currentTransform;
    swapchainInfo.minImageCount = imageCount; // The number of images that our swap chain can use
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.clipped = VK_TRUE; // If external window/image/object placed on our current image, shoudl we sitll draw or clip our image

    QueueFamilyIndices indices = getQueueFamilyIndices(mainDevice.physicalDevice);
    // If graphics and presentation are different then swapchain must let images be shared between families
    // Although this is not that efficient

    if(indices.graphicsFamily != indices.presentationFamily) {
        uint32_t familyIndices[2] = {
            static_cast<uint32_t>(indices.graphicsFamily),
            static_cast<uint32_t>(indices.presentationFamily)
        };

        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = familyIndices;
    } else {
        uint32_t familyIndices[1] = {
            static_cast<uint32_t>(indices.graphicsFamily),
        };

        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 1;
        swapchainInfo.pQueueFamilyIndices = familyIndices;
    }

    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapchainInfo, nullptr, &Swapchain);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    swapChainExtent = extent;
    swapChainImageFormat = surfaceFormat.format;

    std::vector<VkImage> images;
    uint32_t pSwapchainImageCount;

    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, Swapchain, &pSwapchainImageCount, nullptr);
    images.reserve(pSwapchainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, Swapchain, &pSwapchainImageCount, images.data());

    for(VkImage& image : images) {
        SwapChainImage myImage = {};
        myImage.image = image;
        myImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        swapChainImages.push_back(myImage);
    }

    return;
}
void VulkanRenderer::createGraphicsPipeline() {
    FileContent fragmentShaderCode = readFile(ASSET_DIR  "/assets/shaders/shader.frag.spv");
    FileContent vertexShaderCode = readFile(ASSET_DIR  "/assets/shaders/shader.vert.spv");

    // Building shader module to link to graphics pipeline
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    // Creating pipeline
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    // Pipeline implementation

    // 1st stage
    // Vertex Input (Vertex Description is defined here)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Description about data like data spacing, stride etc
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Data format we are using and where to bind it to/from
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // 2nd Stage
    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyInfo.primitiveRestartEnable = VK_FALSE; // allow overriding of strip topology to start new drawing

    // 3rd stage
    // viewport and scissor
    VkViewport viewport = {0.0, 0.0, (float)swapChainExtent.width, (float)swapChainExtent.height, 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, swapChainExtent}; // Not cutting anything, everything between 0 0 and swapChainExtent will be visible

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pViewports = &viewport;
    viewportInfo.pScissors = &scissor;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    // Dynamic States
    // There are dynamic states that allow some properties not bake into pipeline but rather allow them to change when required, something like viewport size. Not using them for now but can be used when required

    // std::vector<VkDynamicState> dynamicStates;

    // dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT); // Can resize viewport with VkCmdSetViewport(commandBuffer, 0, 1, &viewportStruct)
    // dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

    // VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    // dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    // dynamicStateInfo.pDynamicStates = dynamicStates.data();

    // 4th Stage
    // Rasterisor
    // INput assembly changes vertex into primitives, then this rasterizor convert primitives into fragments
    // Fragment is info required to fill a pixel on screen, i.e. fragment contains information like which pixel to fill, and which color to use


    // These modules are not required anymore, they need to be deleted
    vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.image = image;
    imageViewInfo.format = format;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewInfo.subresourceRange.aspectMask = aspect;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1; // mip levels
    imageViewInfo.subresourceRange.baseArrayLayer = 0; // mip levels
    imageViewInfo.subresourceRange.layerCount = 1; // mip levels

    VkImageView imageView;
    vkCreateImageView(mainDevice.logicalDevice, &imageViewInfo, nullptr, &imageView);

    return imageView;
}
VkShaderModule VulkanRenderer::createShaderModule(const FileContent& shaderBinary) {
  VkShaderModuleCreateInfo shaderInfo = {};
  shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderInfo.codeSize = shaderBinary.size;
  shaderInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBinary.content);

  VkShaderModule shaderModule;
  VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderInfo, nullptr, &shaderModule);

  if(result != VK_SUCCESS) {
      throw std::runtime_error("Unable to create shader module");
  }

  return shaderModule;
}
