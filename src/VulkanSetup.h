//
// Created by menegais on 10/11/2020.
//

#ifndef VULKANBASE_VULKANSETUP_H
#define VULKANBASE_VULKANSETUP_H


#include "VulkanStructures.h"

class VulkanSetup {
public:
    void
    vulkanSetup(GLFWwindow *glfWwindow, VulkanHandles &vulkanHandles, PhysicalDeviceInfo &physicalDeviceInfo,
                PresentationEngineInfo &presentationEngineInfo);

private:
    std::vector<const char *> instanceExtensions = {};

    std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


    std::vector<VkExtensionProperties> vulkanQueryInstanceExtensions();

    std::vector<VkExtensionProperties> vulkanQueryDeviceExtensions(VkPhysicalDevice const vkPhysicalDevice);

    std::vector<VkLayerProperties> vulkanQueryInstanceLayers();

    bool
    vulkanValidateLayers(const std::vector<const char *> requiredLayers, const std::vector<VkLayerProperties> layers);

    bool vulkanValidateExtensions(const std::vector<const char *> requiredExtensions,
                                  const std::vector<VkExtensionProperties> extensions);

    bool vulkanPrepareForCreateInstance();

    VkInstance vulkanCreateInstance();

    VkDevice vulkanCreateLogicalDevice(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo);

    VkPhysicalDevice
    vulkanQueryPhysicalDevice(const VulkanHandles vulkanHandles, PhysicalDeviceInfo &physicalDeviceInfo);

    int vulkanScorePhysicalDevices(const PhysicalDeviceInfo physicalDeviceInfo);

    QueueFamilyInfo vulkanGetQueueFamilyInfo(VkPhysicalDevice const vkPhysicalDevice, VkSurfaceKHR const vkSurfaceKhr,
                                             const PhysicalDeviceInfo physicalDeviceInfo);

    void vulkanGetPhysicalDevicesInfo(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKhr,
                                      PhysicalDeviceInfo *physicalDeviceInfo);

    VkSurfaceKHR vulkanCreateSurface(VkInstance const instance, GLFWwindow *window);

    PresentationEngineInfo
    vulkanGetPresentationEngineInfo(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo);

    VkExtent2D
    vulkanGetSwapchainImageExtent(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo);

    VkSurfaceFormatKHR
    vulkanGetSwapchainImageFormat(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo);

    VkPresentModeKHR
    vulkanGetSwapchainPresentMode(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo);

    VkSwapchainKHR vulkanCreateSwapchain(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo,
                                         const PresentationEngineInfo presentationEngineInfo);
};


#endif //VULKANBASE_VULKANSETUP_H
