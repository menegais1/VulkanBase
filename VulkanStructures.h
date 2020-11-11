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
};

struct PhysicalDeviceInfo {
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    QueueFamilyInfo queueFamilyInfo;
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

struct RenderizationStructures {
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;
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

#endif