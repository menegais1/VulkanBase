#include <iostream>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>
#include <set>
#include <fstream>
#include <cmath>
#include "VulkanStructures.h"
#include "VulkanSetup.h"
#include "FileManagers/Bitmap/Bitmap.h"
#include "FileManagers/FileLoader.h"
#include "VulkanHelpers.h"
#include <glm/vec3.hpp>

int const WIDTH = 300;
int const HEIGHT = 300;

struct InputVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
};


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
                                VkDescriptorSetLayout descriptorSetLayout, VkPipelineLayout vkPipelineLayout,
                                const VkShaderModule vertexShaderModule,
                                const VkShaderModule fragmentShaderModule) {



    /////// Define Vertex Input
    VkVertexInputBindingDescription vkVertexInputBindingDescription{};
    vkVertexInputBindingDescription.binding = 0;
    vkVertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vkVertexInputBindingDescription.stride = sizeof(InputVertex);

    VkVertexInputAttributeDescription vkVertexAttrPosition{};
    vkVertexAttrPosition.binding = 0;
    vkVertexAttrPosition.offset = offsetof(struct InputVertex, position);
    vkVertexAttrPosition.format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexAttrPosition.location = 0;

    VkVertexInputAttributeDescription vkVertexAttrColor{};
    vkVertexAttrColor.binding = 0;
    vkVertexAttrColor.offset = offsetof(struct InputVertex, color);
    vkVertexAttrColor.format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexAttrColor.location = 1;

    VkVertexInputAttributeDescription vkVertexAttrTexCoord{};
    vkVertexAttrTexCoord.binding = 0;
    vkVertexAttrTexCoord.offset = offsetof(struct InputVertex, texCoord);
    vkVertexAttrTexCoord.format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexAttrTexCoord.location = 2;

    std::vector<VkVertexInputAttributeDescription> vkDescriptions{vkVertexAttrPosition, vkVertexAttrColor,
                                                                  vkVertexAttrTexCoord};
    VkPipelineVertexInputStateCreateInfo vkVertexInputStateCreateInfo{};
    vkVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vkDescriptions.size();
    vkVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vkVertexInputStateCreateInfo.pVertexBindingDescriptions = &vkVertexInputBindingDescription;
    vkVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkDescriptions.data();

    ///////
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


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan HelloTriangle", nullptr, nullptr);
    VulkanHandles vulkanHandles{};
    PhysicalDeviceInfo physicalDeviceInfo;
    PresentationEngineInfo presentationEngineInfo;
    SwapchainReferences swapchainReferences{};
    VkCommandPool vkGraphicsPool, vkTransferPool;
    VkSemaphore getImageSemaphore{}, presentImageSemaphore{};
    VkQueue graphicsQueue, presentationQueue, transferQueue;

    VulkanSetup vulkanSetup;
    vulkanSetup.vulkanSetup(window, vulkanHandles, physicalDeviceInfo, presentationEngineInfo);

    vkGraphicsPool = CommandBufferUtils::vulkanCreateCommandPool(vulkanHandles,
                                                                 physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex);
    vkTransferPool = CommandBufferUtils::vulkanCreateCommandPool(vulkanHandles,
                                                                 physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex);
    swapchainReferences.images = vulkanGetSwapchainImages(vulkanHandles, presentationEngineInfo);
    swapchainReferences.imageViews = vulkanCreateSwapchainImageViews(vulkanHandles, presentationEngineInfo,
                                                                     swapchainReferences.images);
    vulkanHandles.renderPass = vulkanCreateRenderPass(vulkanHandles, presentationEngineInfo);
    auto vert = vulkanLoadShader("../src/Shaders/vert.spv");
    auto frag = vulkanLoadShader("../src/Shaders/frag.spv");
    auto vertModule = vulkanCreateShaderModule(vulkanHandles, vert);
    auto fragModule = vulkanCreateShaderModule(vulkanHandles, frag);

    VkDescriptorSetLayoutBinding layoutBinding0{};
    layoutBinding0.binding = 0;
    layoutBinding0.descriptorCount = 1;
    layoutBinding0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBinding0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &layoutBinding0;
    VkDescriptorSetLayout vkDescriptorSetLayout0;
    VK_ASSERT(vkCreateDescriptorSetLayout(vulkanHandles.device, &descriptorSetLayoutInfo, nullptr,
                                          &vkDescriptorSetLayout0));

    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{};
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout0;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout vkPipelineLayout;
    VK_ASSERT(vkCreatePipelineLayout(vulkanHandles.device, &vkPipelineLayoutCreateInfo, nullptr, &vkPipelineLayout));

    vulkanHandles.pipeline = vulkanCreatePipeline(vulkanHandles, presentationEngineInfo, vkDescriptorSetLayout0,
                                                  vkPipelineLayout, vertModule, fragModule);
    getImageSemaphore = vulkanCreateSemaphore(vulkanHandles);
    presentImageSemaphore = vulkanCreateSemaphore(vulkanHandles);

    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex, 0,
                     &graphicsQueue);
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex, 0,
                     &presentationQueue);
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex, 0,
                     &transferQueue);

    TransferStructure transferStructure{};
    transferStructure.transferBuffer = CommandBufferUtils::vulkanCreateCommandBuffers(vulkanHandles, vkTransferPool,
                                                                                      1)[0];
    transferStructure.transferAvailableFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);

    VkRect2D viewRect{};
    viewRect.extent = presentationEngineInfo.extents;
    viewRect.offset = {0, 0};
    VkClearValue vkClearValue = {1.0, 0.0, 0.0, 1.0};
    VkFence vkFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);

    std::vector<InputVertex> vertexBufferData{
            {glm::vec3(0, 0, 0),     glm::vec3(1, 0, 0), glm::vec2(0, 1)},
            {glm::vec3(0.5, 0, 0),   glm::vec3(1, 1, 0), glm::vec2(1, 1)},
            {glm::vec3(0.5, 0.5, 0), glm::vec3(1, 0, 1), glm::vec2(1, 0)},
            {glm::vec3(0, 0.5, 0),   glm::vec3(1, 0, 1), glm::vec2(0, 0)}
    };

    std::vector<uint32_t> indexData = {
            0, 1, 2, 0, 2, 3
    };

    Buffer stagingBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                   sizeof(InputVertex) * vertexBufferData.size(),
                                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vulkanMapMemoryWithFlush(vulkanHandles, stagingBuffer, vertexBufferData.data());


    Buffer vertexBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                  sizeof(InputVertex) * vertexBufferData.size(),
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CommandBufferUtils::vulkanBeginCommandBuffer(vulkanHandles, transferStructure.transferBuffer,
                                                 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                                 {transferStructure.transferAvailableFence});
    {
        VkBufferCopy vkBufferCopy{};
        vkBufferCopy.size = vertexBuffer.size;
        vkBufferCopy.srcOffset = 0;
        vkBufferCopy.dstOffset = 0;
        vkCmdCopyBuffer(transferStructure.transferBuffer, stagingBuffer.buffer, vertexBuffer.buffer, 1, &vkBufferCopy);
        VkBufferMemoryBarrier vkBufferMemoryBarrier{};
        vkBufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        vkBufferMemoryBarrier.buffer = vertexBuffer.buffer;
        vkBufferMemoryBarrier.size = VK_WHOLE_SIZE;
        vkBufferMemoryBarrier.offset = 0;
        vkBufferMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        vkBufferMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        vkBufferMemoryBarrier.srcQueueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex;
        vkBufferMemoryBarrier.dstQueueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex;
        vkCmdPipelineBarrier(transferStructure.transferBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0,
                             0, nullptr, 1, &vkBufferMemoryBarrier, 0, nullptr);
    }
    CommandBufferUtils::vulkanSubmitCommandBuffer(transferQueue, transferStructure.transferBuffer,
                                                  std::vector<VkSemaphore>(), std::vector<VkSemaphore>(), nullptr,
                                                  transferStructure.transferAvailableFence);

    Bitmap *image = new Bitmap(FileLoader::getPath("Resources/dog.bmp"));

    VkImageCreateInfo textureCreateInfo{};
    textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    textureCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    textureCreateInfo.extent = {(uint32_t) image->width, (uint32_t) image->height, 1};
    textureCreateInfo.arrayLayers = 1;
    textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureCreateInfo.mipLevels = 1;
    VkImage texture;
    VK_ASSERT(vkCreateImage(vulkanHandles.device, &textureCreateInfo, nullptr, &texture));
    VkMemoryRequirements imageMemoryRequirements = vulkanGetImageMemoryRequirements(vulkanHandles, texture);
    VkDeviceMemory imageDeviceMemory = vulkanAllocateDeviceMemory(vulkanHandles, physicalDeviceInfo,
                                                                  imageMemoryRequirements,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_ASSERT(vkBindImageMemory(vulkanHandles.device, texture, imageDeviceMemory, 0));

    VkImageViewCreateInfo vkImageViewCreateInfo{};
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image = texture;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vkImageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkImageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                        VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkImageView textureImageView;
    VK_ASSERT(vkCreateImageView(vulkanHandles.device, &vkImageViewCreateInfo, nullptr, &textureImageView));

    VkSamplerCreateInfo textureSamplerInfo{};
    textureSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    textureSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureSamplerInfo.anisotropyEnable = VK_FALSE;
    textureSamplerInfo.compareEnable = VK_FALSE;
    textureSamplerInfo.minFilter = VK_FILTER_LINEAR;
    textureSamplerInfo.magFilter = VK_FILTER_LINEAR;
    textureSamplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler textureSampler;
    VK_ASSERT(vkCreateSampler(vulkanHandles.device, &textureSamplerInfo, nullptr, &textureSampler));
    stagingBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo, image->width * image->height * 4 * 4,
                                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vulkanMapMemoryWithFlush(vulkanHandles, stagingBuffer, image->originalBitmapArray);

    CommandBufferUtils::vulkanBeginCommandBuffer(vulkanHandles, transferStructure.transferBuffer,
                                                 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                                 {transferStructure.transferAvailableFence});
    {
        VkImageMemoryBarrier vkImageMemoryBarrier{};
        vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vkImageMemoryBarrier.image = texture;
        vkImageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(transferStructure.transferBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr,
                             1, &vkImageMemoryBarrier);
        VkImageSubresourceLayers vkImageSubresourceLayers = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        VkBufferImageCopy vkBufferImageCopy{};
        vkBufferImageCopy.imageExtent = {(uint32_t) image->width, (uint32_t) image->height, 1};
        vkBufferImageCopy.bufferImageHeight = 0;
        vkBufferImageCopy.bufferRowLength = 0;
        vkBufferImageCopy.bufferOffset = 0;
        vkBufferImageCopy.imageOffset = {0, 0, 0};
        vkBufferImageCopy.imageSubresource = vkImageSubresourceLayers;

        vkCmdCopyBufferToImage(transferStructure.transferBuffer, stagingBuffer.buffer, texture,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &vkBufferImageCopy);
        vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vkImageMemoryBarrier.image = texture;
        vkImageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdPipelineBarrier(transferStructure.transferBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr,
                             1, &vkImageMemoryBarrier);
    }
    CommandBufferUtils::vulkanSubmitCommandBuffer(transferQueue, transferStructure.transferBuffer,
                                                  std::vector<VkSemaphore>(), std::vector<VkSemaphore>(), nullptr,
                                                  transferStructure.transferAvailableFence);

    VkDescriptorPoolSize vkDescriptorPoolSize{};
    vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize.descriptorCount = 1;
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo{};
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.poolSizeCount = 1;
    vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
    vkDescriptorPoolCreateInfo.maxSets = 1;
    VkDescriptorPool imageSamplerPool;
    VK_ASSERT(vkCreateDescriptorPool(vulkanHandles.device, &vkDescriptorPoolCreateInfo, nullptr, &imageSamplerPool));

    VkDescriptorSetAllocateInfo textureDescriptorAllocate{};
    textureDescriptorAllocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    textureDescriptorAllocate.descriptorPool = imageSamplerPool;
    textureDescriptorAllocate.descriptorSetCount = 1;
    textureDescriptorAllocate.pSetLayouts = &vkDescriptorSetLayout0;
    VkDescriptorSet textureDescriptorSet;
    VK_ASSERT(vkAllocateDescriptorSets(vulkanHandles.device, &textureDescriptorAllocate, &textureDescriptorSet));

    VkDescriptorImageInfo vkDescriptorImageInfo{};
    vkDescriptorImageInfo.imageView = textureImageView;
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.sampler = textureSampler;

    VkWriteDescriptorSet vkWriteDescriptorSet{};
    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet.pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet.dstSet = textureDescriptorSet;
    vkWriteDescriptorSet.dstBinding = 0;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.pBufferInfo = nullptr;
    vkWriteDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(vulkanHandles.device, 1, &vkWriteDescriptorSet, 0, nullptr);
    Buffer indexBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                 sizeof(uint32_t) * indexData.size(),
                                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vulkanMapMemoryWithFlush(vulkanHandles, indexBuffer, indexData.data());

    int renderFramesAmount = 2;
    std::vector<RenderFrame> renderFrames(renderFramesAmount);
    for (int i = 0; i < renderFramesAmount; ++i) {
        renderFrames[i] = createRenderFrame(vulkanHandles, vkGraphicsPool);
    }
    float frameNumber = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float flash = std::abs(std::sin(frameNumber / 500.f));
        vkClearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

        for (int i = 0; i < renderFramesAmount; ++i) {
            RenderFrame &renderFrame = renderFrames[i];
            CommandBufferUtils::vulkanWaitForFences(vulkanHandles, {renderFrame.bufferFinishedFence});
            unsigned int imageIndex = 0;
            vkAcquireNextImageKHR(vulkanHandles.device, vulkanHandles.swapchain, UINT64_MAX,
                                  renderFrame.imageReadySemaphore,
                                  VK_NULL_HANDLE,
                                  &imageIndex);

            if (renderFrame.frameBuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(vulkanHandles.device, renderFrame.frameBuffer, nullptr);
            }
            renderFrame.frameBuffer = vulkanCreateFrameBuffer(vulkanHandles,
                                                              presentationEngineInfo.extents.width,
                                                              presentationEngineInfo.extents.height,
                                                              swapchainReferences.imageViews[imageIndex]);

            VkRenderPassBeginInfo vkRenderPassBeginInfo{};
            vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            vkRenderPassBeginInfo.renderPass = vulkanHandles.renderPass;
            vkRenderPassBeginInfo.framebuffer = renderFrame.frameBuffer;
            vkRenderPassBeginInfo.renderArea = viewRect;
            vkRenderPassBeginInfo.clearValueCount = 1;
            vkRenderPassBeginInfo.pClearValues = &vkClearValue;

            CommandBufferUtils::vulkanBeginCommandBuffer(vulkanHandles, renderFrame.commandBuffer, 0);
            {
                vkCmdBeginRenderPass(renderFrame.commandBuffer, &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanHandles.pipeline);
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(renderFrame.commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
                vkCmdBindIndexBuffer(renderFrame.commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0,
                                        1,
                                        &textureDescriptorSet, 0,
                                        nullptr);
                vkCmdDrawIndexed(renderFrame.commandBuffer, indexData.size(), 1, 0, 0, 1);
                vkCmdEndRenderPass(renderFrame.commandBuffer);
            }
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            CommandBufferUtils::vulkanSubmitCommandBuffer(graphicsQueue, renderFrame.commandBuffer,
                                                          {renderFrame.imageReadySemaphore},
                                                          {renderFrame.presentationReadySemaphore}, &waitStage,
                                                          renderFrame.bufferFinishedFence);

            VkPresentInfoKHR vkPresentInfoKhr{};
            vkPresentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            vkPresentInfoKhr.waitSemaphoreCount = 1;
            vkPresentInfoKhr.pWaitSemaphores = &renderFrame.presentationReadySemaphore;
            vkPresentInfoKhr.swapchainCount = 1;
            vkPresentInfoKhr.pSwapchains = &vulkanHandles.swapchain;
            vkPresentInfoKhr.pImageIndices = &imageIndex;

            VK_ASSERT(vkQueuePresentKHR(presentationQueue, &vkPresentInfoKhr));

            frameNumber++;
        }
    }

    vkDestroySwapchainKHR(vulkanHandles.device, vulkanHandles.swapchain, nullptr);
    vkDestroyDevice(vulkanHandles.device, nullptr);
    vkDestroySurfaceKHR(vulkanHandles.instance, vulkanHandles.surface, nullptr);
    vkDestroyInstance(vulkanHandles.instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}