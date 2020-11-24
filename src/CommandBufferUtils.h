//
// Created by menegais on 24/11/2020.
//

#ifndef VULKANBASE_COMMANDBUFFERUTILS_H
#define VULKANBASE_COMMANDBUFFERUTILS_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include "VulkanStructures.h"
#include <vector>
#include <iostream>

class CommandBufferUtils {
public:

    static VkCommandPool vulkanCreateCommandPool(const VulkanHandles vulkanHandles, const int queueFamilyIndex);

    static std::vector<VkCommandBuffer> vulkanCreateCommandBuffers(const VulkanHandles vulkanHandles,
                                                                   VkCommandPool const vkCommandPool,
                                                                   const int commandBufferCount);

    /*
     * Begin a command buffer and wait for fences, if any provided, reset fences automatically
     */
    static void vulkanBeginCommandBuffer(const VulkanHandles vulkanHandles, VkCommandBuffer commandBuffer,
                                         VkCommandBufferUsageFlags flags,
                                         std::vector<VkFence> fence = std::vector<VkFence>(0),
                                         bool resetFences = true);

    /*
     * End and submit a single command buffer
     */
    static void
    vulkanSubmitCommandBuffer(VkQueue queue,
                              VkCommandBuffer commandBuffer,
                              std::vector<VkSemaphore> waitSemaphores,
                              std::vector<VkSemaphore> signalSemaphores,
                              VkPipelineStageFlags *waitDstStageFlags, VkFence fence);

    static void
    vulkanWaitForFences(const VulkanHandles vulkanHandles, std::vector<VkFence> fences, bool resetFences = true);
};


#endif //VULKANBASE_COMMANDBUFFERUTILS_H
