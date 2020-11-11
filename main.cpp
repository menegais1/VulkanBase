#include <iostream>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>
#include <set>
#include <fstream>
#include <cmath>

#define VK_ASSERT(VK_RESULT) if(VK_RESULT != VK_SUCCESS) throw std::runtime_error("ERROR ON VKRESULT");


int const WIDTH = 300;
int const HEIGHT = 300;

std::vector<const char *> instanceExtensions = {};

std::vector<const char *> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
};

std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


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

std::vector<VkExtensionProperties> vulkanQueryInstanceExtensions() {
    unsigned int extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

std::vector<VkExtensionProperties> vulkanQueryDeviceExtensions(const VkPhysicalDevice vkPhysicalDevice) {
    unsigned int extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, extensions.data());

    return extensions;
}

std::vector<VkLayerProperties> vulkanQueryInstanceLayers() {
    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    return layers;
}

bool vulkanValidateLayers(const std::vector<const char *> requiredLayers, const std::vector<VkLayerProperties> layers) {
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

bool vulkanValidateExtensions(const std::vector<const char *> requiredExtensions,
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

bool vulkanPrepareForCreateInstance() {
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

VkInstance vulkanCreateInstance() {
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

VkSurfaceKHR vulkanCreateSurface(const VkInstance instance, GLFWwindow *window) {
    VkSurfaceKHR vkSurfaceKhr;
    VK_ASSERT(glfwCreateWindowSurface(instance, window, nullptr, &vkSurfaceKhr));
    return vkSurfaceKhr;
}

void getPhysicalDevicesInfo(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKhr,
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
    physicalDeviceInfo->physicalDeviceFeatures = vkPhysicalDeviceFeatures;
    physicalDeviceInfo->physicalDeviceProperties = vkPhysicalDeviceProperties;
    physicalDeviceInfo->surfaceCapabilities = vkSurfaceCapabilities;
    physicalDeviceInfo->surfaceFormats = vkSurfaceFormat;
    physicalDeviceInfo->queueFamilyProperties = vkQueueFamilyProperties;
    physicalDeviceInfo->surfacePresentMode = vkSurfacePresentMode;
}

QueueFamilyInfo getQueueFamilyInfo(const VkPhysicalDevice vkPhysicalDevice, const VkSurfaceKHR vkSurfaceKhr,
                                   const PhysicalDeviceInfo physicalDeviceInfo) {
    QueueFamilyInfo queueFamilyInfo;
    for (int i = 0; i < physicalDeviceInfo.queueFamilyProperties.size(); ++i) {
        if (physicalDeviceInfo.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyInfo.graphicsFamilyIndex = i;
        VkBool32 hasPresentationCapability = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurfaceKhr, &hasPresentationCapability);
        if (hasPresentationCapability) queueFamilyInfo.presentationFamilyIndex = i;
    }
    return queueFamilyInfo;
}

int scorePhysicalDevices(const PhysicalDeviceInfo physicalDeviceInfo) {
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

VkPhysicalDevice vulkanQueryPhysicalDevice(const VulkanHandles vulkanHandles, PhysicalDeviceInfo &physicalDeviceInfo) {
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
        getPhysicalDevicesInfo(physicalDevices[i], vulkanHandles.surface, &physicalDeviceInfoList[i]);
        physicalDeviceInfoList[i].queueFamilyInfo = getQueueFamilyInfo(physicalDevices[i], vulkanHandles.surface,
                                                                       physicalDeviceInfoList[i]);
        int score = scorePhysicalDevices(physicalDeviceInfoList[i]);
        if (score > lastScore) {
            lastScore = score;
            lastScoreIndex = i;
        }
    }
    physicalDeviceInfo.queueFamilyInfo = physicalDeviceInfoList[lastScoreIndex].queueFamilyInfo;
    physicalDeviceInfo.queueFamilyProperties = physicalDeviceInfoList[lastScoreIndex].queueFamilyProperties;
    physicalDeviceInfo.physicalDeviceProperties = physicalDeviceInfoList[lastScoreIndex].physicalDeviceProperties;
    physicalDeviceInfo.physicalDeviceFeatures = physicalDeviceInfoList[lastScoreIndex].physicalDeviceFeatures;
    physicalDeviceInfo.surfaceFormats = physicalDeviceInfoList[lastScoreIndex].surfaceFormats;
    physicalDeviceInfo.surfaceCapabilities = physicalDeviceInfoList[lastScoreIndex].surfaceCapabilities;
    physicalDeviceInfo.surfacePresentMode = physicalDeviceInfoList[lastScoreIndex].surfacePresentMode;

    return physicalDevices[lastScoreIndex];
}

VkDevice vulkanCreateLogicalDevice(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    float priority = 1.00;

    std::set<int> queueFamilyIndex = {physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex,
                                      physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex};
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

    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};

    VkDeviceCreateInfo vkDeviceCreateInfo{};
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pEnabledFeatures = &vkPhysicalDeviceFeatures;
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

VkCommandPool vulkanCreateCommandPool(const VulkanHandles vulkanHandles, const int queueFamilyIndex) {
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo{};
    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool vkCommandPool;
    VK_ASSERT(vkCreateCommandPool(vulkanHandles.device, &vkCommandPoolCreateInfo, nullptr, &vkCommandPool));
    return vkCommandPool;
}

std::vector<VkCommandBuffer>
vulkanCreateCommandBuffers(const VulkanHandles vulkanHandles, const VkCommandPool vkCommandPool,
                           const int commandBufferCount) {
    std::vector<VkCommandBuffer> commandBuffers(commandBufferCount);
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo{};
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.commandBufferCount = commandBufferCount;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer vkCommandBuffer;
    VK_ASSERT(vkAllocateCommandBuffers(vulkanHandles.device, &vkCommandBufferAllocateInfo, commandBuffers.data()));
    return commandBuffers;
}

VkFramebuffer
vulkanCreateFrameBuffer(const VulkanHandles vulkanHandles, const PresentationEngineInfo presentationEngineInfo,
                        const VkImageView imageView) {
    VkFramebufferCreateInfo vkFramebufferCreateInfo{};
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.attachmentCount = 1;
    vkFramebufferCreateInfo.pAttachments = &imageView;
    vkFramebufferCreateInfo.renderPass = vulkanHandles.renderPass;
    vkFramebufferCreateInfo.width = presentationEngineInfo.extents.width;
    vkFramebufferCreateInfo.height = presentationEngineInfo.extents.height;
    vkFramebufferCreateInfo.layers = 1;
    VkFramebuffer vkFramebuffer;
    VK_ASSERT(vkCreateFramebuffer(vulkanHandles.device, &vkFramebufferCreateInfo, nullptr, &vkFramebuffer));
    return vkFramebuffer;
}

VkPresentModeKHR
vulkanGetSwapchainPresentMode(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    for (auto presentMode: physicalDeviceInfo.surfacePresentMode) {
        if (presentMode == VK_PRESENT_MODE_FIFO_KHR) return presentMode;
    }
    return physicalDeviceInfo.surfacePresentMode[0];
}

VkSurfaceFormatKHR
vulkanGetSwapchainImageFormat(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    for (auto imageFormat: physicalDeviceInfo.surfaceFormats) {
        if (imageFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            imageFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            return imageFormat;
    }
    return physicalDeviceInfo.surfaceFormats[0];
}

VkExtent2D
vulkanGetSwapchainImageExtent(const VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    return physicalDeviceInfo.surfaceCapabilities.minImageExtent;
}

PresentationEngineInfo
vulkanGetPresentationEngineInfo(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo) {
    PresentationEngineInfo presentationEngineInfo{};
    presentationEngineInfo.format = vulkanGetSwapchainImageFormat(vulkanHandles, physicalDeviceInfo);
    presentationEngineInfo.presentMode = vulkanGetSwapchainPresentMode(vulkanHandles, physicalDeviceInfo);
    presentationEngineInfo.extents = vulkanGetSwapchainImageExtent(vulkanHandles, physicalDeviceInfo);
    return presentationEngineInfo;
}

VkSwapchainKHR vulkanCreateSwapchain(VulkanHandles vulkanHandles, const PhysicalDeviceInfo physicalDeviceInfo,
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


std::vector<VkImage>
vulkanGetSwapchainImages(const VulkanHandles vulkanHandles, PresentationEngineInfo &presentationEngineInfo) {
    unsigned int imageCount = 0;
    vkGetSwapchainImagesKHR(vulkanHandles.device, vulkanHandles.swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(vulkanHandles.device, vulkanHandles.swapchain, &imageCount, images.data());
    presentationEngineInfo.imageCount = imageCount;
    return images;
}

std::vector<VkImageView> vulkanCreateSwapchainImageViews(const VulkanHandles vulkanHandles,
                                                         const PresentationEngineInfo presentationEngineInfo,
                                                         const std::vector<VkImage> images) {

    std::vector<VkImageView> imageViews(presentationEngineInfo.imageCount);
    for (int i = 0; i < presentationEngineInfo.imageCount; ++i) {
        VkImageViewCreateInfo vkImageViewCreateInfo{};
        vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vkImageViewCreateInfo.image = images[i];
        vkImageViewCreateInfo.format = presentationEngineInfo.format.format;
        vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        vkImageViewCreateInfo.subresourceRange.layerCount = 1;
        vkImageViewCreateInfo.subresourceRange.levelCount = 1;
        vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VK_ASSERT(vkCreateImageView(vulkanHandles.device, &vkImageViewCreateInfo, nullptr,
                                    &imageViews[i]));
    }
    return imageViews;
}

std::vector<VkFramebuffer> vulkanCreateSwapchainFrameBuffers(const VulkanHandles vulkanHandles,
                                                             const PresentationEngineInfo presentationEngineInfo,
                                                             const std::vector<VkImageView> imageViews) {
    std::vector<VkFramebuffer> frameBuffers(presentationEngineInfo.imageCount);
    for (int i = 0; i < presentationEngineInfo.imageCount; ++i) {
        frameBuffers[i] = vulkanCreateFrameBuffer(vulkanHandles, presentationEngineInfo, imageViews[i]);
    }
    return frameBuffers;
}

std::vector<VkCommandBuffer> vulkanCreateSwapchainCommandBuffers(const VulkanHandles vulkanHandles,
                                                                 const PresentationEngineInfo presentationEngineInfo,
                                                                 const VkCommandPool vkCommandPool) {
    return vulkanCreateCommandBuffers(vulkanHandles, vkCommandPool, presentationEngineInfo.imageCount);
}

std::vector<char> vulkanLoadShader(const std::string filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "CANNOT OPEN SHADER FILE" << std::endl;
        exit(-1);
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

VkShaderModule vulkanCreateShaderModule(const VulkanHandles vulkanHandles, const std::vector<char> shaderBytes) {
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo{};
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = nullptr;
    vkShaderModuleCreateInfo.codeSize = shaderBytes.size();
    vkShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(shaderBytes.data());

    VkShaderModule shaderModule;
    VK_ASSERT(vkCreateShaderModule(vulkanHandles.device, &vkShaderModuleCreateInfo, nullptr, &shaderModule));
    return shaderModule;
}

VkRenderPass
vulkanCreateRenderPass(const VulkanHandles vulkanHandles, const PresentationEngineInfo presentationEngineInfo) {
    VkAttachmentDescription vkAttachmentDescription{};
    vkAttachmentDescription.format = presentationEngineInfo.format.format;
    vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentReference vkAttachmentReference{};
    vkAttachmentReference.attachment = 0;
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription vkSubpassDescription{};
    vkSubpassDescription.colorAttachmentCount = 1;
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = nullptr;
    vkSubpassDescription.pDepthStencilAttachment = nullptr;
    vkSubpassDescription.pPreserveAttachments = nullptr;
    vkSubpassDescription.pResolveAttachments = nullptr;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency vkSubpassDependency{};
    vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    vkSubpassDependency.dstSubpass = 0;
    vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.srcAccessMask = 0;
    vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    vkSubpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo vkRenderPassCreateInfo{};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.attachmentCount = 1;
    vkRenderPassCreateInfo.pAttachments = &vkAttachmentDescription;
    vkRenderPassCreateInfo.dependencyCount = 1;
    vkRenderPassCreateInfo.pDependencies = &vkSubpassDependency;
    VkRenderPass vkRenderPass;
    VK_ASSERT(vkCreateRenderPass(vulkanHandles.device, &vkRenderPassCreateInfo, nullptr, &vkRenderPass));
    return vkRenderPass;
}

VkPipeline vulkanCreatePipeline(const VulkanHandles vulkanHandles, const PresentationEngineInfo presentationEngineInfo,
                                const VkShaderModule vertexShaderModule,
                                const VkShaderModule fragmentShaderModule) {

    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{};
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.setLayoutCount = 0;
    vkPipelineLayoutCreateInfo.pSetLayouts = nullptr;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout vkPipelineLayout;
    VK_ASSERT(vkCreatePipelineLayout(vulkanHandles.device, &vkPipelineLayoutCreateInfo, nullptr, &vkPipelineLayout));

    VkPipelineVertexInputStateCreateInfo vkVertexInputStateCreateInfo{};
    vkVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vkVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vkVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
    vkVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo{};
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport vkViewport{};
    vkViewport.width = presentationEngineInfo.extents.width;
    vkViewport.height = presentationEngineInfo.extents.height;
    vkViewport.x = vkViewport.y = 0;
    vkViewport.minDepth = 0.0;
    vkViewport.maxDepth = 1.0;

    VkRect2D vkScissor{};
    vkScissor.offset = {0, 0};
    vkScissor.extent = presentationEngineInfo.extents;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo{};
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.viewportCount = 1;
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    vkPipelineViewportStateCreateInfo.pScissors = &vkScissor;

    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo{};
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0;
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0;
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0;

    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo{};
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    //LET THE REST AS DEFAULT

    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState{};
    vkPipelineColorBlendAttachmentState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vkPipelineColorBlendAttachmentState.blendEnable = VK_FALSE;


    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo{};
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = 1;
    vkPipelineColorBlendStateCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo{};
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;


    VkPipelineShaderStageCreateInfo vkVertexShaderStageCreateInfo{};
    vkVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkVertexShaderStageCreateInfo.pName = "main";
    vkVertexShaderStageCreateInfo.module = vertexShaderModule;

    VkPipelineShaderStageCreateInfo vkFragmentShaderStageCreateInfo{};
    vkFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkFragmentShaderStageCreateInfo.pName = "main";
    vkFragmentShaderStageCreateInfo.module = fragmentShaderModule;

    VkPipelineShaderStageCreateInfo stages[2]{vkVertexShaderStageCreateInfo, vkFragmentShaderStageCreateInfo};
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{};
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.renderPass = vulkanHandles.renderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0;
    vkGraphicsPipelineCreateInfo.stageCount = 2;
    vkGraphicsPipelineCreateInfo.pStages = stages;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = nullptr;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;

    VkPipeline vkPipeline;
    VK_ASSERT(vkCreateGraphicsPipelines(vulkanHandles.device, VK_NULL_HANDLE, 1,
                                        &vkGraphicsPipelineCreateInfo, nullptr, &vkPipeline));
    return vkPipeline;
}

VkSemaphore vulkanCreateSemaphore(VulkanHandles vulkanHandles) {
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo{};
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore vkSemaphore;
    VK_ASSERT(vkCreateSemaphore(vulkanHandles.device, &vkSemaphoreCreateInfo, nullptr, &vkSemaphore));
    return vkSemaphore;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan HelloTriangle", nullptr, nullptr);
    VulkanHandles vulkanHandles{};
    PhysicalDeviceInfo physicalDeviceInfo;
    PresentationEngineInfo presentationEngineInfo;
    RenderizationStructures renderizationStructures{};
    VkCommandPool vkGraphicsPool;
    VkCommandPool vkPresentationPool;
    VkCommandBuffer vkPresentationBuffer;
    VkSemaphore getImageSemaphore{};
    VkSemaphore presentImageSemaphore{};
    VkQueue graphicsQueue, presentationQueue;
    vulkanHandles.instance = vulkanCreateInstance();
    vulkanHandles.surface = vulkanCreateSurface(vulkanHandles.instance, window);
    vulkanHandles.physicalDevice = vulkanQueryPhysicalDevice(vulkanHandles, physicalDeviceInfo);
    vulkanHandles.device = vulkanCreateLogicalDevice(vulkanHandles, physicalDeviceInfo);
    vkGraphicsPool = vulkanCreateCommandPool(vulkanHandles, physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex);
    vkPresentationPool = vulkanCreateCommandPool(vulkanHandles,
                                                 physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex);
    vkPresentationBuffer = vulkanCreateCommandBuffers(vulkanHandles, vkPresentationPool, 1)[0];
    presentationEngineInfo = vulkanGetPresentationEngineInfo(vulkanHandles, physicalDeviceInfo);
    vulkanHandles.swapchain = vulkanCreateSwapchain(vulkanHandles, physicalDeviceInfo, presentationEngineInfo);
    renderizationStructures.images = vulkanGetSwapchainImages(vulkanHandles, presentationEngineInfo);
    renderizationStructures.imageViews = vulkanCreateSwapchainImageViews(vulkanHandles, presentationEngineInfo,
                                                                         renderizationStructures.images);
    renderizationStructures.commandBuffers = vulkanCreateSwapchainCommandBuffers(vulkanHandles, presentationEngineInfo,
                                                                                 vkGraphicsPool);
    vulkanHandles.renderPass = vulkanCreateRenderPass(vulkanHandles, presentationEngineInfo);
    renderizationStructures.frameBuffers = vulkanCreateSwapchainFrameBuffers(vulkanHandles, presentationEngineInfo,
                                                                             renderizationStructures.imageViews);

    auto vert = vulkanLoadShader("../Shaders/vert.spv");
    auto frag = vulkanLoadShader("../Shaders/frag.spv");
    auto vertModule = vulkanCreateShaderModule(vulkanHandles, vert);
    auto fragModule = vulkanCreateShaderModule(vulkanHandles, frag);
    vulkanHandles.pipeline = vulkanCreatePipeline(vulkanHandles, presentationEngineInfo, vertModule, fragModule);
    getImageSemaphore = vulkanCreateSemaphore(vulkanHandles);
    presentImageSemaphore = vulkanCreateSemaphore(vulkanHandles);

    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex, 0,
                     &graphicsQueue);
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex, 0,
                     &presentationQueue);


    VkRect2D viewRect{};
    viewRect.extent = presentationEngineInfo.extents;
    viewRect.offset = {0, 0};
    VkClearValue vkClearValue = {1.0, 0.0, 0.0, 1.0};
    VkFence vkFence{};
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    //we want to create the fence with the Create  Signaled flag, so we can wait on it before using it on a gpu command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_ASSERT(vkCreateFence(vulkanHandles.device, &fenceCreateInfo, nullptr, &vkFence));
    float frameNumber = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float flash = std::abs(std::sin(frameNumber / 500.f));
        vkClearValue.color = {{0.0f, 0.0f, flash, 1.0f}};
        VK_ASSERT(vkWaitForFences(vulkanHandles.device, 1, &vkFence, true, UINT64_MAX));
        VK_ASSERT(vkResetFences(vulkanHandles.device, 1, &vkFence));

        unsigned int imageIndex = 0;
        vkAcquireNextImageKHR(vulkanHandles.device, vulkanHandles.swapchain, UINT64_MAX, getImageSemaphore,
                              VK_NULL_HANDLE,
                              &imageIndex);
        VkRenderPassBeginInfo vkRenderPassBeginInfo{};
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.renderPass = vulkanHandles.renderPass;
        vkRenderPassBeginInfo.framebuffer = renderizationStructures.frameBuffers[imageIndex];
        vkRenderPassBeginInfo.renderArea = viewRect;
        vkRenderPassBeginInfo.clearValueCount = 1;
        vkRenderPassBeginInfo.pClearValues = &vkClearValue;

        VkCommandBufferBeginInfo vkCommandBufferBeginInfo{};
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkCommandBuffer graphicsBuffer = renderizationStructures.commandBuffers[imageIndex];
        vkBeginCommandBuffer(graphicsBuffer, &vkCommandBufferBeginInfo);
        vkCmdBeginRenderPass(graphicsBuffer, &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(graphicsBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanHandles.pipeline);
        vkCmdDraw(graphicsBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(graphicsBuffer);
        vkEndCommandBuffer(graphicsBuffer);

        VkSubmitInfo vkSubmitInfo{};
        vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkSubmitInfo.commandBufferCount = 1;
        vkSubmitInfo.pCommandBuffers = &graphicsBuffer;
        vkSubmitInfo.waitSemaphoreCount = 1;
        vkSubmitInfo.pWaitSemaphores = &getImageSemaphore;
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores = &presentImageSemaphore;
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        vkSubmitInfo.pWaitDstStageMask = &waitStage;
        VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &vkSubmitInfo, vkFence));


        VkPresentInfoKHR vkPresentInfoKhr{};
        vkPresentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        vkPresentInfoKhr.waitSemaphoreCount = 1;
        vkPresentInfoKhr.pWaitSemaphores = &presentImageSemaphore;
        vkPresentInfoKhr.swapchainCount = 1;
        vkPresentInfoKhr.pSwapchains = &vulkanHandles.swapchain;
        vkPresentInfoKhr.pImageIndices = &imageIndex;

        VK_ASSERT(vkQueuePresentKHR(presentationQueue, &vkPresentInfoKhr));
        frameNumber++;
    }

    vkDestroySwapchainKHR(vulkanHandles.device, vulkanHandles.swapchain, nullptr);
    vkDestroyDevice(vulkanHandles.device, nullptr);
    vkDestroySurfaceKHR(vulkanHandles.instance, vulkanHandles.surface, nullptr);
    vkDestroyInstance(vulkanHandles.instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
