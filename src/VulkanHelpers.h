//
// Created by menegais on 22/11/2020.
//

#ifndef VULKANBASE_VULKANHELPERS_H
#define VULKANBASE_VULKANHELPERS_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include "VulkanStructures.h"

VkMemoryRequirements vulkanGetBufferMemoryRequirements(VulkanHandles vulkanHandles, VkBuffer vkBuffer) {
    VkMemoryRequirements vkMemoryRequirements{};
    vkGetBufferMemoryRequirements(vulkanHandles.device, vkBuffer, &vkMemoryRequirements);
    return vkMemoryRequirements;
}

VkMemoryRequirements vulkanGetImageMemoryRequirements(VulkanHandles vulkanHandles, VkImage vkImage) {
    VkMemoryRequirements vkMemoryRequirements{};
    vkGetImageMemoryRequirements(vulkanHandles.device, vkImage, &vkMemoryRequirements);
    return vkMemoryRequirements;
}

int vulkanGetMemoryTypeIndex(PhysicalDeviceInfo physicalDeviceInfo, uint32_t memoryTypeBits,
                             VkMemoryPropertyFlagBits flagBits) {
    for (int i = 0; i < physicalDeviceInfo.memoryProperties.memoryTypeCount; ++i) {
        VkMemoryType memoryType = physicalDeviceInfo.memoryProperties.memoryTypes[i];
        if ((memoryTypeBits & (1 << i)) && (memoryType.propertyFlags & flagBits) == flagBits) {
            return i;
        }
    }
    return -1;
}


VkSemaphore vulkanCreateSemaphore(VulkanHandles vulkanHandles) {
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo{};
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore vkSemaphore;
    VK_ASSERT(vkCreateSemaphore(vulkanHandles.device, &vkSemaphoreCreateInfo, nullptr, &vkSemaphore));
    return vkSemaphore;
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
    VK_ASSERT(vkAllocateCommandBuffers(vulkanHandles.device, &vkCommandBufferAllocateInfo, commandBuffers.data()));
    return commandBuffers;
}

VkFramebuffer vulkanCreateFrameBuffer(const VulkanHandles vulkanHandles, uint32_t width, uint32_t height,
                                      const VkImageView imageView) {
    VkFramebufferCreateInfo vkFramebufferCreateInfo{};
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.attachmentCount = 1;
    vkFramebufferCreateInfo.pAttachments = &imageView;
    vkFramebufferCreateInfo.renderPass = vulkanHandles.renderPass;
    vkFramebufferCreateInfo.width = width;
    vkFramebufferCreateInfo.height = height;
    vkFramebufferCreateInfo.layers = 1;
    VkFramebuffer vkFramebuffer;
    VK_ASSERT(vkCreateFramebuffer(vulkanHandles.device, &vkFramebufferCreateInfo, nullptr, &vkFramebuffer));
    return vkFramebuffer;
}

VkFence vulkanCreateFence(VulkanHandles vulkanHandles, VkFenceCreateFlags flags) {
    VkFence vkFence{};
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = flags;
    VK_ASSERT(vkCreateFence(vulkanHandles.device, &fenceCreateInfo, nullptr, &vkFence));
    return vkFence;
}

RenderFrame createRenderFrame(VulkanHandles vulkanHandles, VkCommandPool vkCommandPool) {
    RenderFrame renderFrame{};
    renderFrame.commandBuffer = vulkanCreateCommandBuffers(vulkanHandles, vkCommandPool, 1)[0];
    renderFrame.imageReadySemaphore = vulkanCreateSemaphore(vulkanHandles);
    renderFrame.presentationReadySemaphore = vulkanCreateSemaphore(vulkanHandles);
    renderFrame.bufferFinishedFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);
    renderFrame.frameBuffer = VK_NULL_HANDLE;
    return renderFrame;
}


VkBuffer vulkanAllocateExclusiveBuffer(VulkanHandles vulkanHandles, uint32_t size, VkBufferUsageFlags usageFlags) {
    VkBuffer buffer;
    VkBufferCreateInfo vkVertexBufferCreateInfo{};
    vkVertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkVertexBufferCreateInfo.size = size;
    vkVertexBufferCreateInfo.usage = usageFlags;
    vkVertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_ASSERT(vkCreateBuffer(vulkanHandles.device, &vkVertexBufferCreateInfo, nullptr, &buffer));
    return buffer;
}

VkDeviceMemory
vulkanAllocateDeviceMemory(VulkanHandles vulkanHandles, PhysicalDeviceInfo physicalDeviceInfo,
                           VkMemoryRequirements vkMemoryRequirements,
                           VkMemoryPropertyFlagBits memoryPropertyFlags) {

    VkMemoryAllocateInfo vkMemoryAllocateInfo{};
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.memoryTypeIndex = vulkanGetMemoryTypeIndex(physicalDeviceInfo,
                                                                    vkMemoryRequirements.memoryTypeBits,
                                                                    memoryPropertyFlags);
    VkDeviceMemory vkDeviceMemory{};
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    VK_ASSERT(vkAllocateMemory(vulkanHandles.device, &vkMemoryAllocateInfo, nullptr,
                               &vkDeviceMemory));
    return vkDeviceMemory;

}

void vulkanMapMemoryWithFlush(VulkanHandles vulkanHandles, Buffer buffer, void *data) {
    void *memoryPointer;
    VK_ASSERT(vkMapMemory(vulkanHandles.device, buffer.deviceMemory, 0, buffer.size, 0,
                          &memoryPointer));

    memcpy(memoryPointer, data, buffer.size);

    VkMappedMemoryRange vkMappedMemoryRange{};
    vkMappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    vkMappedMemoryRange.memory = buffer.deviceMemory;
    vkMappedMemoryRange.size = VK_WHOLE_SIZE;
    vkMappedMemoryRange.offset = 0;

    vkFlushMappedMemoryRanges(vulkanHandles.device, 1, &vkMappedMemoryRange);
    memoryPointer = nullptr;
    vkUnmapMemory(vulkanHandles.device, buffer.deviceMemory);

}

Buffer allocateExclusiveBuffer(VulkanHandles vulkanHandles, PhysicalDeviceInfo physicalDeviceInfo, uint32_t size,
                               VkBufferUsageFlags usageFlags, VkMemoryPropertyFlagBits memoryPropertyFlags) {
    Buffer buffer{};
    buffer.size = size;
    buffer.buffer = vulkanAllocateExclusiveBuffer(vulkanHandles, size, usageFlags);
    buffer.memoryRequirements = vulkanGetBufferMemoryRequirements(vulkanHandles, buffer.buffer);
    buffer.deviceMemory = vulkanAllocateDeviceMemory(vulkanHandles, physicalDeviceInfo, buffer.memoryRequirements,
                                                     memoryPropertyFlags);

    VK_ASSERT(vkBindBufferMemory(vulkanHandles.device, buffer.buffer, buffer.deviceMemory, 0));

    return buffer;
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

#endif //VULKANBASE_VULKANHELPERS_H
