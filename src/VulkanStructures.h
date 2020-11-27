//
// Created by menegais on 10/11/2020.
//

#ifndef VULKANBASE_VULKANSTRUCTURE_H
#define VULKANBASE_VULKANSTRUCTURE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>

#define VK_ASSERT(VK_RESULT) if(VK_RESULT != VK_SUCCESS) throw std::runtime_error("ERROR ON VKRESULT");

struct QueueFamilyInfo {
    int graphicsFamilyIndex = -1;
    int presentationFamilyIndex = -1;
    int transferFamilyIndex = -1;
};

struct PhysicalDeviceInfo {
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    QueueFamilyInfo queueFamilyInfo;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkPresentModeKHR> surfacePresentMode;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
};

struct PresentationEngineInfo {
    unsigned int imageCount;
    VkSurfaceFormatKHR format;
    VkExtent2D extents;
    VkPresentModeKHR presentMode;
};

struct RenderFrame {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageReadySemaphore;
    VkSemaphore presentationReadySemaphore;
    VkFramebuffer frameBuffer;
    VkFence bufferFinishedFence;
};

struct SwapchainReferences {
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
};

struct CommandBufferStructure {
    VkCommandBuffer commandBuffer;
    VkFence bufferAvaibleFence;
    VkQueue queue;
    uint32_t queueFamilyIndex;
};

struct VulkanHandles {
    VkInstance instance;
    VkDevice device;
    VkPipeline pipeline;
    VkRenderPass renderPass;
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
};

struct Buffer {
    VkBuffer buffer;
    VkDeviceSize size;
    VkMemoryRequirements memoryRequirements;
    VkDeviceMemory deviceMemory;
};

struct Texture2D {
    void* data;
    uint32_t width;
    uint32_t height;
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VkMemoryRequirements memoryRequirements;
    VkDeviceMemory deviceMemory;
};


#endif