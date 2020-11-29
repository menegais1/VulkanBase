//
// Created by menegais on 10/11/2020.
//

#include <vector>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstring>
#include <stdexcept>
#include <set>
#include <iostream>
#include "VulkanSetup.h"
#include "VulkanStructures.h"
#include "VulkanDebug.h"


std::vector<VkExtensionProperties> VulkanSetup::vulkanQueryInstanceExtensions() {
    unsigned int extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

std::vector<VkExtensionProperties> VulkanSetup::vulkanQueryDeviceExtensions(const VkPhysicalDevice vkPhysicalDevice) {
    unsigned int extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, extensions.data());

    return extensions;
}

std::vector<VkLayerProperties> VulkanSetup::vulkanQueryInstanceLayers() {
    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    return layers;
}

bool VulkanSetup::vulkanValidateLayers(const std::vector<const char *> requiredLayers,
                                       const std::vector<VkLayerProperties> layers) {
    bool present = false;
    for (int i = 0; i < requiredLayers.size(); ++i) {
        for (int j = 0; j < layers.size(); ++j) {
            if (strcmp(requiredLayers[i], layers[j].layerName) == 0) {
                present = true;
                break;
            }
            present = false;
        }
        if (!present) return false;
    }
    return true;
}

bool VulkanSetup::vulkanValidateExtensions(const std::vector<const char *> requiredExtensions,
                                           const std::vector<VkExtensionProperties> extensions) {
    bool present = false;
    for (int i = 0; i < requiredExtensions.size(); ++i) {
        for (int j = 0; j < extensions.size(); ++j) {
            if (strcmp(requiredExtensions[i], extensions[j].extensionName) == 0) {
                present = true;
                break;
            }
            present = false;
        }
        if (!present) return false;
    }
    return true;
}

bool VulkanSetup::vulkanPrepareForCreateInstance() {
    unsigned int count = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    for (int i = 0; i < count; ++i) {
        instanceExtensions.push_back(glfwExtensions[i]);
    }
    auto extensions = vulkanQueryInstanceExtensions();
    auto layers = vulkanQueryInstanceLayers();
    bool layersPresent = vulkanValidateLayers(instanceLayers, layers);
    bool extensionsPresent = vulkanValidateExtensions(instanceExtensions, extensions);
    return layersPresent && extensionsPresent;
}

VkInstance VulkanSetup::vulkanCreateInstance() {
    if (!vulkanPrepareForCreateInstance()) {
        throw std::runtime_error("Missing layers or extension in instance");
    }
    VkApplicationInfo vkApplicationInfo{};
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_2;
    vkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vkApplicationInfo.pApplicationName = "Hello Triangle Vulkan";
    vkApplicationInfo.pEngineName = "None";
    VkInstanceCreateInfo vkInstanceCreateInfo{};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    vkInstanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    vkInstanceCreateInfo.enabledLayerCount = instanceLayers.size();
    vkInstanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
    VkInstance instance;
    VK_ASSERT(vkCreateInstance(&vkInstanceCreateInfo, nullptr, &instance));
    return instance;
}

VkSurfaceKHR VulkanSetup::vulkanCreateSurface(const VkInstance instance, GLFWwindow *window) {
    VkSurfaceKHR vkSurfaceKhr;
    VK_ASSERT(glfwCreateWindowSurface(instance, window, nullptr, &vkSurfaceKhr));
    return vkSurfaceKhr;
}

void VulkanSetup::vulkanGetPhysicalDevicesInfo(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKhr,
                                               PhysicalDeviceInfo *physicalDeviceInfo) {
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    unsigned int surfaceFormatCount = 0;
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurfaceKhr, &vkSurfaceCapabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceKhr, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> vkSurfaceFormat(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceKhr, &surfaceFormatCount, vkSurfaceFormat.data());
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
    unsigned int presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurfaceKhr, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> vkSurfacePresentMode(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurfaceKhr, &presentModeCount,
                                              vkSurfacePresentMode.data());
    unsigned int queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertiesCount, nullptr);
    std::vector<VkQueueFamilyProperties> vkQueueFamilyProperties(queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertiesCount,
                                             vkQueueFamilyProperties.data());

    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);

    physicalDeviceInfo->memoryProperties = vkPhysicalDeviceMemoryProperties;
    physicalDeviceInfo->physicalDeviceFeatures = vkPhysicalDeviceFeatures;
    physicalDeviceInfo->physicalDeviceProperties = vkPhysicalDeviceProperties;
    physicalDeviceInfo->surfaceCapabilities = vkSurfaceCapabilities;
    physicalDeviceInfo->surfaceFormats = vkSurfaceFormat;
    physicalDeviceInfo->queueFamilyProperties = vkQueueFamilyProperties;
    physicalDeviceInfo->surfacePresentMode = vkSurfacePresentMode;
}

QueueFamilyInfo
VulkanSetup::vulkanGetQueueFamilyInfo(const VkPhysicalDevice vkPhysicalDevice, const VkSurfaceKHR vkSurfaceKhr,
                                      const PhysicalDeviceInfo physicalDeviceInfo) {
    QueueFamilyInfo queueFamilyInfo;
    for (int i = 0; i < physicalDeviceInfo.queueFamilyProperties.size(); ++i) {
        if (physicalDeviceInfo.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyInfo.graphicsFamilyIndex = i;
        if (physicalDeviceInfo.queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            queueFamilyInfo.transferFamilyIndex = i;

        VkBool32 hasPresentationCapability = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurfaceKhr, &hasPresentationCapability);
        if (hasPresentationCapability) queueFamilyInfo.presentationFamilyIndex = i;

        printQueueFamilies(physicalDeviceInfo.queueFamilyProperties[i], hasPresentationCapability);
    }
    return queueFamilyInfo;
}

int VulkanSetup::vulkanScorePhysicalDevices(const PhysicalDeviceInfo physicalDeviceInfo) {
    int score = 0;

    if (physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex == -1) {
        score = -1;
        return score;
    }

    if (physicalDeviceInfo.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 10;
    } else {
        score += 5;
    }

    return score;
}

VkPhysicalDevice
VulkanSetup::vulkanQueryPhysicalDevice(const VulkanHandles vulkanHandles, PhysicalDeviceInfo &physicalDeviceInfo) {
    unsigned int physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(vulkanHandles.instance, &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(vulkanHandles.instance, &physicalDevicesCount, physicalDevices.data());

    int lastScore = 0;
    int lastScoreIndex = -1;
    std::vector<PhysicalDeviceInfo> physicalDeviceInfoList(physicalDevicesCount);
    for (int i = 0; i < physicalDevicesCount; ++i) {
        auto extensions = vulkanQueryDeviceExtensions(physicalDevices[i]);
        if (!vulkanValidateExtensions(deviceExtensions, extensions)) {
            break;
        }
        vulkanGetPhysicalDevicesInfo(physicalDevices[i], vulkanHandles.surface, &physicalDeviceInfoList[i]);
        physicalDeviceInfoList[i].queueFamilyInfo = vulkanGetQueueFamilyInfo(physicalDevices[i], vulkanHandles.surface,
                                                                             physicalDeviceInfoList[i]);
        int score = vulkanScorePhysicalDevices(physicalDeviceInfoList[i]);
        if (score > lastScore) {
            lastScore = score;
            lastScoreIndex = i;
        }
    }
    physicalDeviceInfo.memoryProperties = physicalDeviceInfoList[lastScoreIndex].memoryProperties;
    physicalDeviceInfo.queueFamilyInfo = physicalDeviceInfoList[lastScoreIndex].queueFamilyInfo;
    physicalDeviceInfo.queueFamilyProperties = physicalDeviceInfoList[lastScoreIndex].queueFamilyProperties;
    physicalDeviceInfo.physicalDeviceProperties = physicalDeviceInfoList[lastScoreIndex].physicalDeviceProperties;
    physicalDeviceInfo.physicalDeviceFeatures = physicalDeviceInfoList[lastScoreIndex].physicalDeviceFeatures;
    physicalDeviceInfo.surfaceFormats = physicalDeviceInfoList[lastScoreIndex].surfaceFormats;
    physicalDeviceInfo.surfaceCapabilities = physicalDeviceInfoList[lastScoreIndex].surfaceCapabilities;
    physicalDeviceInfo.surfacePresentMode = physicalDeviceInfoList[lastScoreIndex].surfacePresentMode;

    return physicalDevices[lastScoreIndex];
}

VkDevice
VulkanSetup::vulkanCreateLogicalDevice(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    float priority = 1.00;

    std::set<int> queueFamilyIndex = {physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex,
                                      physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex,
                                      physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo[queueFamilyIndex.size()];
    int count = 0;
    for (auto index : queueFamilyIndex) {
        vkDeviceQueueCreateInfo[count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        vkDeviceQueueCreateInfo[count].pNext = nullptr;
        vkDeviceQueueCreateInfo[count].flags = 0;
        vkDeviceQueueCreateInfo[count].queueFamilyIndex = index;
        vkDeviceQueueCreateInfo[count].pQueuePriorities = &priority;
        vkDeviceQueueCreateInfo[count].queueCount = 1;
        count++;
    }

    if(physicalDeviceInfo.physicalDeviceFeatures.tessellationShader == VK_TRUE){
        std::cout << "TESSELATION IS PRESENT" << std::endl;
    }

    VkDeviceCreateInfo vkDeviceCreateInfo{};
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pEnabledFeatures = &physicalDeviceInfo.physicalDeviceFeatures;
    vkDeviceCreateInfo.queueCreateInfoCount = queueFamilyIndex.size();
    vkDeviceCreateInfo.pQueueCreateInfos = vkDeviceQueueCreateInfo;
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = nullptr;
    vkDeviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    vkDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    VkDevice vkDevice;
    VK_ASSERT(vkCreateDevice(vulkanHandles.physicalDevice, &vkDeviceCreateInfo, nullptr, &vkDevice));
    return vkDevice;
}


PresentationEngineInfo
VulkanSetup::vulkanGetPresentationEngineInfo(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    PresentationEngineInfo presentationEngineInfo{};
    presentationEngineInfo.format = vulkanGetSwapchainImageFormat(vulkanHandles, physicalDeviceInfo);
    presentationEngineInfo.presentMode = vulkanGetSwapchainPresentMode(vulkanHandles, physicalDeviceInfo);
    presentationEngineInfo.extents = vulkanGetSwapchainImageExtent(vulkanHandles, physicalDeviceInfo);
    return presentationEngineInfo;
}


VkPresentModeKHR
VulkanSetup::vulkanGetSwapchainPresentMode(const VulkanHandles vulkanHandles,
                                           const PhysicalDeviceInfo physicalDeviceInfo) {
    for (auto presentMode: physicalDeviceInfo.surfacePresentMode) {
        if (presentMode == VK_PRESENT_MODE_FIFO_KHR) return presentMode;
    }
    return physicalDeviceInfo.surfacePresentMode[0];
}

VkSurfaceFormatKHR
VulkanSetup::vulkanGetSwapchainImageFormat(const VulkanHandles vulkanHandles,
                                           const PhysicalDeviceInfo physicalDeviceInfo) {
    for (auto imageFormat: physicalDeviceInfo.surfaceFormats) {
        if (imageFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            imageFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            return imageFormat;
    }
    return physicalDeviceInfo.surfaceFormats[0];
}

VkExtent2D
VulkanSetup::vulkanGetSwapchainImageExtent(const VulkanHandles vulkanHandles,
                                           const PhysicalDeviceInfo physicalDeviceInfo) {
    return physicalDeviceInfo.surfaceCapabilities.minImageExtent;
}


void
VulkanSetup::vulkanSetup(GLFWwindow *glfWwindow, VulkanHandles &vulkanHandles, PhysicalDeviceInfo &physicalDeviceInfo,
                         PresentationEngineInfo &presentationEngineInfo) {
    vulkanHandles.instance = vulkanCreateInstance();
    vulkanHandles.surface = vulkanCreateSurface(vulkanHandles.instance, glfWwindow);
    vulkanHandles.physicalDevice = vulkanQueryPhysicalDevice(vulkanHandles, physicalDeviceInfo);
    vulkanHandles.device = vulkanCreateLogicalDevice(vulkanHandles, physicalDeviceInfo);
    presentationEngineInfo = vulkanGetPresentationEngineInfo(vulkanHandles, physicalDeviceInfo);
    vulkanHandles.swapchain = vulkanCreateSwapchain(vulkanHandles, physicalDeviceInfo, presentationEngineInfo);

}


VkSwapchainKHR VulkanSetup::vulkanCreateSwapchain(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo,
                                     const PresentationEngineInfo presentationEngineInfo) {

    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKhr{};
    vkSwapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;
    vkSwapchainCreateInfoKhr.surface = vulkanHandles.surface;
    vkSwapchainCreateInfoKhr.presentMode = presentationEngineInfo.presentMode;
    vkSwapchainCreateInfoKhr.imageFormat = presentationEngineInfo.format.format;
    vkSwapchainCreateInfoKhr.imageColorSpace = presentationEngineInfo.format.colorSpace;
    vkSwapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkSwapchainCreateInfoKhr.minImageCount = physicalDeviceInfo.surfaceCapabilities.minImageCount;
    vkSwapchainCreateInfoKhr.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    vkSwapchainCreateInfoKhr.imageArrayLayers = 1;
    vkSwapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKhr.clipped = VK_TRUE;
    vkSwapchainCreateInfoKhr.imageExtent = presentationEngineInfo.extents;

    unsigned int queueFamilyIndices[2] = {
            (unsigned int) physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex,
            (unsigned int) physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex};
    if (physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex !=
        physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex) {
        vkSwapchainCreateInfoKhr.queueFamilyIndexCount = 2;
        vkSwapchainCreateInfoKhr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vkSwapchainCreateInfoKhr.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        vkSwapchainCreateInfoKhr.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkSwapchainCreateInfoKhr.queueFamilyIndexCount = 0;
        vkSwapchainCreateInfoKhr.pQueueFamilyIndices = nullptr;
    }
    VkSwapchainKHR vkSwapchainKhr;
    VK_ASSERT(vkCreateSwapchainKHR(vulkanHandles.device, &vkSwapchainCreateInfoKhr, nullptr, &vkSwapchainKhr));
    return vkSwapchainKhr;

}