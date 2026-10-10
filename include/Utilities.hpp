#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vector>
#include <stdio.h>

struct FileContent {
    size_t size;
    const char* content;

    FileContent() {
        content = nullptr;
        size = 0;
    }

    // 1. Destructor: Automatically frees memory when the variable goes out of scope
    ~FileContent() {
        if (content != nullptr) {
            free((void*)content);
        }
    }
    // 2. Prevent accidental copying which causes double-free bugs
    FileContent(const FileContent&) = delete;
    FileContent& operator=(const FileContent&) = delete;

    // 3. Allow moving memory (required for your value-return function overload)
    FileContent(FileContent&& other) noexcept : content(other.content), size(other.size) {
        other.content = nullptr;
        other.size = 0;
    }
    FileContent& operator=(FileContent&& other) noexcept {
        if (this != &other) {
            if (content != nullptr) free((void*)content);
            content = other.content;
            size = other.size;
            other.content = nullptr;
            other.size = 0;
        }
        return *this;
    }
};

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

static void readFile(const char* filePath, FileContent& item) {
    if (item.content != nullptr) {
        free((void*)item.content);
        item.content = nullptr;
        item.size = 0;
    }

    FILE *file;

    if(fopen_s(&file, filePath, "rb") != 0 || file == nullptr) {
        throw std::runtime_error("Unable to load file");
    }

    fseek(file, 0, SEEK_END);
    item.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* tempBuffer = (char*)malloc(sizeof(char) * (item.size + 1));
    if (tempBuffer == nullptr) {
        fclose(file);
        throw std::runtime_error("Memory allocation to read file failed");
    }

    size_t bytesRead = fread(tempBuffer, sizeof(char), item.size, file);
    tempBuffer[bytesRead] = '\0';

    item.size = bytesRead;
    item.content = tempBuffer;

    fclose(file);

    return ;
}

static FileContent readFile(const char* filePath) {

    FILE *file;

    if(fopen_s(&file, filePath, "rb") != 0 || file == nullptr) {
        throw std::runtime_error(std::string("Unable to load file") + filePath);
    }
    FileContent item;

    fseek(file, 0, SEEK_END);
    item.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* tempBuffer = (char*)malloc(sizeof(char) * (item.size + 1));
    if (tempBuffer == nullptr) {
        fclose(file);
        throw std::runtime_error("Memory allocation to read file failed");
    }


    size_t bytesRead = fread(tempBuffer, sizeof(char), item.size, file);
    tempBuffer[bytesRead] = '\0';

    item.size = bytesRead;
    item.content = tempBuffer;
    fclose(file);

    return item;
}
