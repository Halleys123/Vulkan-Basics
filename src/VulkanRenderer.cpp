#include "VulkanRenderer.hpp"
#include "GLFW/glfw3.h"
#include "Utilities.hpp"

#include <cstddef>
#include <limits>
#include <set>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <array>

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
        createRenderPass();
        createGraphicsPipeline();
    } catch (const std::runtime_error& err) {
        printf("Error: %s\n", err.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::cleanup() {
    vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);
    for(auto i : swapChainFrameBuffers) {
        vkDestroyFramebuffer(mainDevice.logicalDevice, i, nullptr);
    }
    vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);
    vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
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
void VulkanRenderer::recordCommands() {
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Can same command be used multiple times at same time

    VkClearValue clearValues = {
        {0.6f, 0.65f, 0.4f, 1.0f}
    };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    renderPassBeginInfo.pClearValues = &clearValues;
    renderPassBeginInfo.clearValueCount = 1;

    for(size_t i = 0;i < commandBuffer.size();i += 1) {
        VkResult startResult = vkBeginCommandBuffer(commandBuffer[i], &commandBufferBeginInfo);
        if(startResult != VK_SUCCESS) throw std::runtime_error("Failed to start recording command buffer");

        {
            renderPassBeginInfo.framebuffer = swapChainFrameBuffers[i];
            // Steps are render pass
            vkCmdBeginRenderPass(commandBuffer[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(commandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
                vkCmdDraw(commandBuffer[i], 3, 1, 0, 0);
                vkCmdBindPipeline(commandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr);
            }
            vkCmdEndRenderPass(commandBuffer[i]);
        }

        VkResult endResult = vkEndCommandBuffer(commandBuffer[i]);
        if(endResult != VK_SUCCESS) throw std::runtime_error("Failed to end recording command buffer");
    }
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
        queueFamiliesInfo.back().queueCount = 1;
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

    uint32_t familyIndices[2] = {
        static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentationFamily)
    };

    if(indices.graphicsFamily != indices.presentationFamily) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = familyIndices;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
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
    images.resize(pSwapchainImageCount);
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
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // What does depthClampEnable does
    // Any geometry that falls outside the view frustum (closer than the near plane or further than the far plane) is discarded (clipped) and not rendered at all.
    // The Problem: If a large object (like a mountain or a massive spaceship) crosses your far clipping plane, it gets sliced open, creating a jarring, ugly visual hole where you can see inside the 3D model.
    // If you change this to VK_TRUE, Vulkan stops discarding geometry that is too close or too far. Instead, it "clamps" its depth values.
    // Geometry beyond the far plane is projectively squashed and drawn directly onto the far plane instead of being cut off.
    // Turning this on requires a device features to be enabled.
    rasterizationInfo.depthClampEnable = VK_FALSE;
    // If this is turned on this will totally discard the data that is received and we don't want to draw anything to framebuffer
    // So usually its false;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    // Decides how to handle filling points between vertices and is different from VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    // Features other than polygon mode fill requires some other features
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    // Using some value other than 1 we need some GPU features (WIDE_LINES)
    rasterizationInfo.lineWidth = 1.0f;
    // Same as OpenGL culling
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    // To determine to know which side is backside
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;

    // 5th Stage
    // MULTISAMPLING
    // It is a subset of supersampling
    VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Number of samples to use per fragment, only used when MMA is enabled

    // 6th Stage
    // BLENDING
    // If two objects are drawn on top of each other how to handle that part (maybe like glass, or something)
    VkPipelineColorBlendAttachmentState colorState = {};
    // Tell which colors to apply this blend to
    colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorState.blendEnable = VK_TRUE;

    // Blending is made on equation
    // (srcColorBlendFactor * newColor) colorBlendOp (destinationColorBlendFactor * oldColor)
    colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    colorState.colorBlendOp = VK_BLEND_OP_ADD;
    colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo blendingInfo = {};
    blendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // To properly enable blending on something like glass, mathematical operation are used (better option) not logical operations
    blendingInfo.logicOpEnable = VK_FALSE;
    blendingInfo.pAttachments = &colorState;
    blendingInfo.attachmentCount = 1;
    // We pass 4 constants for rgba which are then used in colorState values when something like VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR
    // blendingInfo.blendConstants;

    // DESCRIPTOR_SET are set here
    VkPipelineLayoutCreateInfo vkPipelineLayoutInfo = {};
    vkPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutInfo.setLayoutCount = 0;
    vkPipelineLayoutInfo.pSetLayouts = nullptr;
    vkPipelineLayoutInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutInfo.pPushConstantRanges = nullptr;

    // Create Pipeline Layout
    VkResult pipelineResult = vkCreatePipelineLayout(mainDevice.logicalDevice, &vkPipelineLayoutInfo, nullptr, &pipelineLayout);
    if(pipelineResult != VK_SUCCESS) {
        throw std::runtime_error("Unable to create Pipeline Layout");
    }

    // TODO DEPTH STENCIL TESTING

    // FINALLY GRAPHICS PIPELINE
    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.stageCount = 2;
    graphicsPipelineInfo.pStages = shaderStages;
    graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineInfo.pInputAssemblyState = &assemblyInfo;
    graphicsPipelineInfo.pViewportState = &viewportInfo;
    graphicsPipelineInfo.pDynamicState = nullptr;
    graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
    graphicsPipelineInfo.pMultisampleState = &multisamplingInfo;
    graphicsPipelineInfo.pColorBlendState = &blendingInfo;
    graphicsPipelineInfo.pDepthStencilState = nullptr;
    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.renderPass = renderPass; // Telling pipeline that it (pipeline) will be used by this renderpass
    graphicsPipelineInfo.subpass = 0;

    // This is used to make multiple pipelines (if any) based on some primary pipeline
    // That is one pipeline you fill properties, for other pipelines, tell which properties are different from first one, then vulkan will create those for us
    graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &graphicsPipeline);

    // These modules are not required anymore, they need to be deleted
    vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);

    if(result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create graphics pipeline");
    }
}
void VulkanRenderer::createRenderPass() {
    VkAttachmentDescription attachDescription = {};
    attachDescription.format = swapChainImageFormat;
    attachDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // when we start render pass what to do with the attachment to which we are going to process info to
    attachDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // -------------------------------

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // -------------------------------

    // Need to determine when layout transitions occur using subpass dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies;

    // Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;

    // -------------------------------

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassInfo.pDependencies = subpassDependencies.data();// Ensure attachDescription is in-scope and valid here
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachDescription;

    VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassInfo, nullptr, &renderPass);

    if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}
void VulkanRenderer::createFramebuffers() {
    swapChainFrameBuffers.resize(swapChainImages.size());

    // Creating buffer for each swapchain image
    for(size_t i = 0;i < swapChainFrameBuffers.size();i += 1) {
        std::array<VkImageView, 1> attachments = {
            swapChainImages[i].imageView
        };

        VkFramebufferCreateInfo frameBufferInfo = {};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderPass;
        frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        frameBufferInfo.pAttachments = attachments.data();
        frameBufferInfo.width = swapChainExtent.width;
        frameBufferInfo.height = swapChainExtent.height;
        frameBufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &frameBufferInfo, nullptr, &swapChainFrameBuffers[i]);

        if(result != VK_SUCCESS) throw std::runtime_error("Unable to create framebuffer");
    }
}
void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices indices = getQueueFamilyIndices(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = indices.graphicsFamily;

    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &commandPoolCreateInfo, nullptr, &graphicsCommandPool);

    if(result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create command pool");
    }
}
void VulkanRenderer::createCommandBuffers() {
    commandBuffer.reserve(swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo commandBufferInfo = {}; // Command buffer already exists so rather we use allocate instead of create info
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = graphicsCommandPool;
    // Primary means command can be paswsed to queue
    // a secondary means some other command will call it and can't be passed on to queue directly
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size()); // Total number of command buffers we are going to create

    VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &commandBufferInfo, commandBuffer.data());
    if(result != VK_SUCCESS) throw std::runtime_error("Unable to allocate command buffers");

    recordCommands();
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
