//
// Created by menegais on 24/11/2020.
//

#include "CommandBufferUtils.h"


VkCommandPool
CommandBufferUtils::vulkanCreateCommandPool(const VulkanHandles vulkanHandles, const int queueFamilyIndex) {
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo{};
    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool vkCommandPool;
    VK_ASSERT(vkCreateCommandPool(vulkanHandles.device, &vkCommandPoolCreateInfo, nullptr, &vkCommandPool));
    return vkCommandPool;
}

std::vector<VkCommandBuffer>
CommandBufferUtils::vulkanCreateCommandBuffers(const VulkanHandles vulkanHandles, const VkCommandPool vkCommandPool,
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

void CommandBufferUtils::vulkanBeginCommandBuffer(const VulkanHandles vulkanHandles, VkCommandBuffer commandBuffer,
                                                  VkCommandBufferUsageFlags flags, std::vector<VkFence> fences,
                                                  bool resetFences) {
    VkCommandBufferBeginInfo bufferBegin{};
    bufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBegin.flags = flags;
    vulkanWaitForFences(vulkanHandles, fences, resetFences);
    VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &bufferBegin));
}

void CommandBufferUtils::vulkanSubmitCommandBuffer(VkQueue queue,
                                                   VkCommandBuffer commandBuffer,
                                                   std::vector<VkSemaphore> waitSemaphores,
                                                   std::vector<VkSemaphore> signalSemaphores,
                                                   VkPipelineStageFlags *waitDstStageFlags, VkFence fence) {
    VK_ASSERT(vkEndCommandBuffer(commandBuffer));
    VkSubmitInfo transferSubmitInfo{};
    transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    transferSubmitInfo.commandBufferCount = 1;
    transferSubmitInfo.pCommandBuffers = &commandBuffer;
    transferSubmitInfo.waitSemaphoreCount = waitSemaphores.size();
    transferSubmitInfo.signalSemaphoreCount = signalSemaphores.size();
    transferSubmitInfo.pSignalSemaphores = signalSemaphores.data();
    transferSubmitInfo.pWaitSemaphores = waitSemaphores.data();
    if (waitDstStageFlags != nullptr)
        transferSubmitInfo.pWaitDstStageMask = waitDstStageFlags;

    VK_ASSERT(vkQueueSubmit(queue, 1, &transferSubmitInfo, fence));
}

void CommandBufferUtils::vulkanWaitForFences(const VulkanHandles vulkanHandles, std::vector<VkFence> fences,
                                             bool resetFences) {
    if (!fences.empty()) {
        VK_ASSERT(vkWaitForFences(vulkanHandles.device, fences.size(), fences.data(), VK_TRUE, UINT64_MAX));
        if (resetFences)
            VK_ASSERT(vkResetFences(vulkanHandles.device, 1, fences.data()));
    }
}
