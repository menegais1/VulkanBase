//
// Created by menegais on 16/11/2020.
//

#ifndef VULKANBASE_VULKANDEBUG_H
#define VULKANBASE_VULKANDEBUG_H

#define GLFW_INCLUDE_VULKAN
#define DEBUG

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

std::string to_string(VkQueueFlags flags) {
    std::string str;
    if (flags & VK_QUEUE_GRAPHICS_BIT) str.append("GRAPHICS");
    if (flags & VK_QUEUE_TRANSFER_BIT) str.append(" | TRANSFER");
    if (flags & VK_QUEUE_COMPUTE_BIT) str.append(" | COMPUTE");
    if (flags & VK_QUEUE_PROTECTED_BIT) str.append(" | PROTECTED");
    if (flags & VK_QUEUE_SPARSE_BINDING_BIT) str.append(" | SPARSE BINDING");

    return str;
}

void printQueueFamilies(VkQueueFamilyProperties familyProperties, VkBool32 hasPresentationCapability) {
#ifdef DEBUG
    std::cout << "Queue number: " + std::to_string(familyProperties.queueCount)
              << std::endl;
    std::cout << "Queue flags: " + to_string(familyProperties.queueFlags)
              << (hasPresentationCapability ? " | PRESENTATION" : "") << std::endl;
#endif
}

#endif //VULKANBASE_VULKANDEBUG_H
