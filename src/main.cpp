#include <iostream>

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int const WIDTH = 500;
int const HEIGHT = 500;
#define PI 3.14159265359
struct InputVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

struct MVP {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projetion;
};

struct TessInfo {
    glm::vec3 tessLevelOuter;
    float tessLevelInner;
};

struct TerrainPatch {
    TessInfo tessInfo;
    std::vector<InputVertex> vertices;
    std::vector<uint32_t> indices;
    MVP mvp;
    Buffer vertexBuffer;
    Buffer indexBuffer;

    Buffer tessInfoUniform;
    VkDescriptorSet tessInfoDescriptorSet;

    Buffer mvpUniform;
    VkDescriptorSet mvpDescriptorSet;
};

struct Camera {
    float speed = 2;
    glm::vec3 eye = glm::vec3(0, 0, 1);
    glm::vec3 center = glm::vec3(1, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec2 angle;
    bool isDragging;

    void positionCameraCenter() {
        glm::vec3 direction;
        direction.x = cos(angle.y * PI / 180.0) * cos(angle.x * PI / 180.0);
        direction.y = sin(angle.x * PI / 180.0);
        direction.z = sin(angle.y * PI / 180.0) * cos(angle.x * PI / 180.0);
        center = eye + glm::normalize(direction);
    }
} camera;

struct LightInformation {
    glm::vec3 position;
    glm::vec3 cameraPosition;
} lightInformation;

glm::vec2 lastMousePosition;
float mouseSensitivity = 0.5;
float globalInnerTess = 1;
float globalOuterTess = 1;
int activePatch = 0;
VkPipeline activePipeline;
VkPipeline shadedPipeline;
VkPipeline wirePipeline;
std::vector<TerrainPatch> terrainPatches;
float maxTesselationLevel = -1;
float angle = 0;
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    float moveSpeed = camera.speed * 0.01;

    glm::vec3 forward = glm::normalize(camera.center - camera.eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
    if (key == GLFW_KEY_W) {
        camera.eye = camera.eye + forward * moveSpeed;
        camera.center = camera.center + forward * moveSpeed;
        camera.up = glm::vec3(0, 1, 0);

    } else if (key == GLFW_KEY_S) {
        camera.eye = camera.eye + forward * -moveSpeed;
        camera.center = camera.center + forward * -moveSpeed;
        camera.up = glm::vec3(0, 1, 0);
    } else if (key == GLFW_KEY_D) {
        camera.eye = camera.eye + right * moveSpeed;
        camera.center = camera.center + right * moveSpeed;
        camera.up = glm::vec3(0, 1, 0);

    } else if (key == GLFW_KEY_A) {
        camera.eye = camera.eye + right * -moveSpeed;
        camera.center = camera.center + right * -moveSpeed;
        camera.up = glm::vec3(0, 1, 0);
    } else if (key == GLFW_KEY_Q) {
        camera.eye = camera.eye + glm::vec3(0, 1, 0) * moveSpeed;
        camera.center = camera.center + glm::vec3(0, 1, 0) * moveSpeed;
        camera.up = glm::vec3(0, 1, 0);

    } else if (key == GLFW_KEY_E) {
        camera.eye = camera.eye + glm::vec3(0, 1, 0) * -moveSpeed;
        camera.center = camera.center + glm::vec3(0, 1, 0) * -moveSpeed;
        camera.up = glm::vec3(0, 1, 0);
    }

    if (key == GLFW_KEY_LEFT) {
        terrainPatches[activePatch].tessInfo.tessLevelInner -= 1;
        if (terrainPatches[activePatch].tessInfo.tessLevelInner < 1) terrainPatches[activePatch].tessInfo.tessLevelInner = 1;
    } else if (key == GLFW_KEY_RIGHT) {
        terrainPatches[activePatch].tessInfo.tessLevelInner += 1;
        if (terrainPatches[activePatch].tessInfo.tessLevelInner > maxTesselationLevel) terrainPatches[activePatch].tessInfo.tessLevelInner = maxTesselationLevel;
    }

    if (key == GLFW_KEY_DOWN) {
        globalOuterTess -= 1;
        if (globalOuterTess < 1) globalOuterTess = 1;
    } else if (key == GLFW_KEY_UP) {
        globalOuterTess += 1;
        if (globalOuterTess > maxTesselationLevel) globalOuterTess = maxTesselationLevel;
    }

    if (key == GLFW_KEY_O) {
        activePipeline = wirePipeline;
    } else if (key == GLFW_KEY_P) {
        activePipeline = shadedPipeline;
    }

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9)
        activePatch = key - GLFW_KEY_1;
}

void mouseButton(GLFWwindow *window, int button, int action, int modifier) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        camera.isDragging = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        camera.isDragging = false;
    }
}

void mouseMovement(GLFWwindow *window, double xpos, double ypos) {
    ypos = HEIGHT - ypos;
    if (camera.isDragging) {

        float xDelta = (xpos - lastMousePosition.x);
        float yDelta = (ypos - lastMousePosition.y);
        float xOffset = xDelta * mouseSensitivity;
        float yOffset = yDelta * mouseSensitivity;

        camera.angle.x += yOffset;
        camera.angle.y += xOffset;

        if (camera.angle.x >= 89) {
            camera.angle.x = 89;
        } else if (camera.angle.x <= -89) {
            camera.angle.x = -89;
        }

        camera.positionCameraCenter();
    }
    lastMousePosition = glm::vec2(xpos, ypos);

}

VkRenderPass
vulkanCreateRenderPass(const VulkanHandles vulkanHandles, const PresentationEngineInfo presentationEngineInfo) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = presentationEngineInfo.format.format;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference{};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    VkSubpassDescription vkSubpassDescription{};
    vkSubpassDescription.colorAttachmentCount = 1;
    vkSubpassDescription.pColorAttachments = &colorAttachmentReference;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = nullptr;
    vkSubpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
    vkSubpassDescription.pPreserveAttachments = nullptr;
    vkSubpassDescription.pResolveAttachments = nullptr;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency vkSubpassDependency{};
    vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    vkSubpassDependency.dstSubpass = 0;
    vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    vkSubpassDependency.srcAccessMask = 0;
    vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    vkSubpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo vkRenderPassCreateInfo{};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.attachmentCount = attachments.size();
    vkRenderPassCreateInfo.pAttachments = attachments.data();
    vkRenderPassCreateInfo.dependencyCount = 1;
    vkRenderPassCreateInfo.pDependencies = &vkSubpassDependency;
    VkRenderPass vkRenderPass;
    VK_ASSERT(vkCreateRenderPass(vulkanHandles.device, &vkRenderPassCreateInfo, nullptr, &vkRenderPass));
    return vkRenderPass;
}

VkPipeline vulkanCreatePipeline(const VulkanHandles vulkanHandles, const PresentationEngineInfo presentationEngineInfo, VkPipelineLayout vkPipelineLayout, VkRenderPass renderPass, VkPolygonMode polygonMode,
                                const VkShaderModule vertexShaderModule,
                                const VkShaderModule fragmentShaderModule, const VkShaderModule tesselationControlShaderModule,
                                const VkShaderModule tesselationEvaluationShaderModule, const VkShaderModule geometryShaderModule) {



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

    VkVertexInputAttributeDescription vkVertexAttrTexCoord{};
    vkVertexAttrTexCoord.binding = 0;
    vkVertexAttrTexCoord.offset = offsetof(struct InputVertex, texCoord);
    vkVertexAttrTexCoord.format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexAttrTexCoord.location = 1;

    std::vector<VkVertexInputAttributeDescription> vkDescriptions{vkVertexAttrPosition,
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
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineTessellationStateCreateInfo vkPipelineTessellationStateCreateInfo{};
    vkPipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    vkPipelineTessellationStateCreateInfo.patchControlPoints = 3;

    VkViewport vkViewport{};
    vkViewport.width = presentationEngineInfo.extents.width;
    vkViewport.height = -float(presentationEngineInfo.extents.height);
    vkViewport.x = 0;
    vkViewport.y = presentationEngineInfo.extents.height;
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
    vkPipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
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
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;


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

    VkPipelineShaderStageCreateInfo vkTesselationControlShaderStageCreateInfo{};
    vkTesselationControlShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkTesselationControlShaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    vkTesselationControlShaderStageCreateInfo.pName = "main";
    vkTesselationControlShaderStageCreateInfo.module = tesselationControlShaderModule;

    VkPipelineShaderStageCreateInfo vkTesselationEvaluationShaderStageCreateInfo{};
    vkTesselationEvaluationShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkTesselationEvaluationShaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    vkTesselationEvaluationShaderStageCreateInfo.pName = "main";
    vkTesselationEvaluationShaderStageCreateInfo.module = tesselationEvaluationShaderModule;

    VkPipelineShaderStageCreateInfo vkGeometryShaderStageCreateInfo{};
    vkGeometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkGeometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vkGeometryShaderStageCreateInfo.pName = "main";
    vkGeometryShaderStageCreateInfo.module = geometryShaderModule;

    VkPipelineShaderStageCreateInfo stages[5]{vkVertexShaderStageCreateInfo, vkFragmentShaderStageCreateInfo,
                                              vkTesselationControlShaderStageCreateInfo, vkTesselationEvaluationShaderStageCreateInfo, vkGeometryShaderStageCreateInfo};
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{};
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.renderPass = renderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0;
    vkGraphicsPipelineCreateInfo.stageCount = 5;
    vkGraphicsPipelineCreateInfo.pStages = stages;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = &vkPipelineTessellationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = nullptr;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;

    VkPipeline vkPipeline;
    VK_ASSERT(vkCreateGraphicsPipelines(vulkanHandles.device, VK_NULL_HANDLE, 1,
                                        &vkGraphicsPipelineCreateInfo, nullptr, &vkPipeline));
    return vkPipeline;
}

float round(float var) {
    float value = (int) (var * 100 + .5);
    return (float) value / 100;
}

std::vector<TerrainPatch>
buildTerrainPatches(VulkanHandles vulkanHandles, PhysicalDeviceInfo physicalDeviceInfo, CommandBufferStructure transferStructure,
                    int xAmount, int zAmount, glm::vec3 patchSize, glm::vec3 initialPosition, VkDescriptorSetLayout tessInfoLayout, VkDescriptorSetLayout mvpLayout, VkDescriptorPool descriptorPool) {
    std::vector<TerrainPatch> patches;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    Buffer stagingBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                   sizeof(InputVertex) * 4,
                                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    glm::vec3 totalSize = patchSize * glm::vec3(xAmount, 0, zAmount);
    glm::vec3 uvStep = glm::vec3(1, 1, 1) / glm::vec3(xAmount, 0, zAmount);
    for (int x = 0; x < xAmount; x++) {
        for (int z = 0; z < zAmount; z++) {
            TerrainPatch terrainPatch{};
            glm::vec3 patchSizeXZ = patchSize * glm::vec3(x, 0, z);
            glm::vec3 patchPosition = initialPosition + patchSizeXZ;
            glm::vec3 leftBottom = glm::vec3(-0.5, 0, -0.5);
            glm::vec3 rightTop = glm::vec3(0.5, 0, 0.5);
            glm::vec3 rightBottom = glm::vec3(0.5, 0, -0.5);
            glm::vec3 leftTop = glm::vec3(-0.5, 0, 0.5);

            terrainPatch.vertices = {{leftBottom,  glm::vec2(round(uvStep.x * (x + 0)), round(uvStep.z * (z + 0)))},
                                     {rightBottom, glm::vec2(round(uvStep.x * (x + 1)), round(uvStep.z * (z + 0)))},
                                     {rightTop,    glm::vec2(round(uvStep.x * (x + 1)), round(uvStep.z * (z + 1)))},
                                     {leftTop,     glm::vec2(round(uvStep.x * (x + 0)), round(uvStep.z * (z + 1)))}};
            std::cout << "PATCH{" << std::endl;
            for (int i = 0; i < terrainPatch.vertices.size(); ++i) {
                std::cout << "UV: " << terrainPatch.vertices[i].texCoord.x << " " << terrainPatch.vertices[i].texCoord.y << std::endl;
            }
            std::cout << "}" << std::endl;

            terrainPatch.indices = {0, 1, 2, 0, 2, 3};
            terrainPatch.tessInfo.tessLevelInner = 4;
            terrainPatch.tessInfo.tessLevelOuter = glm::vec3(1, 1, 1);
            terrainPatch.mvp.model = {glm::vec4(patchSize.x, 0, 0, patchPosition.x),
                                      glm::vec4(0, patchSize.y, 0, patchPosition.y),
                                      glm::vec4(0, 0, patchSize.z, patchPosition.z),
                                      glm::vec4(0, 0, 0, 1)};
            terrainPatch.mvp.model = glm::transpose(terrainPatch.mvp.model);

            vulkanMapMemoryWithFlush(vulkanHandles, stagingBuffer, terrainPatch.vertices.data());


            terrainPatch.vertexBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                                sizeof(InputVertex) * terrainPatch.vertices.size(),
                                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            copyBufferHostDevice(vulkanHandles, physicalDeviceInfo, transferStructure, stagingBuffer, terrainPatch.vertexBuffer,
                                 VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                 physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex);

            terrainPatch.indexBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo,
                                                               sizeof(uint32_t) * terrainPatch.indices.size(),
                                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            vulkanMapMemoryWithFlush(vulkanHandles, terrainPatch.indexBuffer, terrainPatch.indices.data());

            terrainPatch.mvpUniform = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo, sizeof(MVP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            terrainPatch.mvpDescriptorSet = vulkanAllocateDescriptorSet(vulkanHandles, descriptorPool, mvpLayout);

            VkDescriptorBufferInfo mvpBufferInfo{};
            mvpBufferInfo.buffer = terrainPatch.mvpUniform.buffer;
            mvpBufferInfo.range = terrainPatch.mvpUniform.size;
            mvpBufferInfo.offset = 0;


            descriptorWrites.push_back(vulkanGetWriteDescriptorSet(vulkanHandles, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, terrainPatch.mvpDescriptorSet, 0, &mvpBufferInfo, nullptr));

            terrainPatch.tessInfoUniform = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo, sizeof(TessInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            terrainPatch.tessInfoDescriptorSet = vulkanAllocateDescriptorSet(vulkanHandles, descriptorPool, tessInfoLayout);

            VkDescriptorBufferInfo tessInfoBufferInfo{};
            tessInfoBufferInfo.buffer = terrainPatch.tessInfoUniform.buffer;
            tessInfoBufferInfo.range = terrainPatch.tessInfoUniform.size;
            tessInfoBufferInfo.offset = 0;
            descriptorWrites.push_back(vulkanGetWriteDescriptorSet(vulkanHandles, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                   terrainPatch.tessInfoDescriptorSet, 0, &tessInfoBufferInfo, nullptr));
            vkUpdateDescriptorSets(vulkanHandles.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
            descriptorWrites.clear();
            patches.push_back(terrainPatch);
        }
    }

    return patches;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan HelloTriangle", nullptr, nullptr);
    glfwSetKeyCallback(window, keyboard);
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetCursorPosCallback(window, mouseMovement);
    VulkanHandles vulkanHandles{};
    PhysicalDeviceInfo physicalDeviceInfo;
    PresentationEngineInfo presentationEngineInfo;
    SwapchainReferences swapchainReferences{};
    VkCommandPool vkGraphicsPool, vkTransferPool;
    VkSemaphore getImageSemaphore{}, presentImageSemaphore{};
    VkQueue graphicsQueue, presentationQueue, transferQueue;

    VulkanSetup vulkanSetup;
    vulkanSetup.vulkanSetup(window, vulkanHandles, physicalDeviceInfo, presentationEngineInfo);
    maxTesselationLevel = physicalDeviceInfo.physicalDeviceProperties.limits.maxTessellationGenerationLevel;
    vkGraphicsPool = CommandBufferUtils::vulkanCreateCommandPool(vulkanHandles,
                                                                 physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex);
    vkTransferPool = CommandBufferUtils::vulkanCreateCommandPool(vulkanHandles,
                                                                 physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex);
    swapchainReferences.images = vulkanGetSwapchainImages(vulkanHandles, presentationEngineInfo);
    swapchainReferences.imageViews = vulkanCreateSwapchainImageViews(vulkanHandles, presentationEngineInfo,
                                                                     swapchainReferences.images);

    VkImage depthMap = vulkanCreateImage2D(vulkanHandles, {WIDTH, HEIGHT}, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkMemoryRequirements depthMapRequirement = vulkanGetImageMemoryRequirements(vulkanHandles, depthMap);
    VkDeviceMemory depthMapMemory = vulkanAllocateDeviceMemory(vulkanHandles, physicalDeviceInfo, depthMapRequirement, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_ASSERT(vkBindImageMemory(vulkanHandles.device, depthMap, depthMapMemory, 0));
    VkImageView depthMapImageView = vulkanCreateImageView2D(vulkanHandles, depthMap, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    VkRenderPass renderPass = vulkanCreateRenderPass(vulkanHandles, presentationEngineInfo);

    auto vert = vulkanLoadShader("../src/Shaders/vert.spv");
    auto frag = vulkanLoadShader("../src/Shaders/frag.spv");
    auto tessControl = vulkanLoadShader("../src/Shaders/tessControl.spv");
    auto tessEval = vulkanLoadShader("../src/Shaders/tessEval.spv");
    auto geometry = vulkanLoadShader("../src/Shaders/geometry.spv");
    auto vertModule = vulkanCreateShaderModule(vulkanHandles, vert);
    auto fragModule = vulkanCreateShaderModule(vulkanHandles, frag);
    auto tessModule = vulkanCreateShaderModule(vulkanHandles, tessControl);
    auto tessEvalModule = vulkanCreateShaderModule(vulkanHandles, tessEval);
    auto geometryModule = vulkanCreateShaderModule(vulkanHandles, geometry);

    VkDescriptorPool descriptorPool = vulkanAllocateDescriptorPool(vulkanHandles,
                                                                   {vulkanAllocateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 24),
                                                                    vulkanAllocateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 24)},
                                                                   24);


    VkDescriptorSetLayout vkDescriptorSetLayout0 = vulkanCreateDescriptorSetLayout(vulkanHandles,
                                                                                   {
                                                                                           vulkanCreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                                                                                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT),
                                                                                           vulkanCreateDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)});
    VkDescriptorSetLayout vkDescriptorSetLayout1 = vulkanCreateDescriptorSetLayout(vulkanHandles,
                                                                                   {vulkanCreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT)});
    VkDescriptorSetLayout vkDescriptorSetLayout2 = vulkanCreateDescriptorSetLayout(vulkanHandles,
                                                                                   {vulkanCreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)});

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {vkDescriptorSetLayout0, vkDescriptorSetLayout1, vkDescriptorSetLayout2};
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{};
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    vkPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout vkPipelineLayout;
    VK_ASSERT(vkCreatePipelineLayout(vulkanHandles.device, &vkPipelineLayoutCreateInfo, nullptr, &vkPipelineLayout));

    shadedPipeline = vulkanCreatePipeline(vulkanHandles, presentationEngineInfo,
                                          vkPipelineLayout, renderPass, VK_POLYGON_MODE_FILL, vertModule, fragModule, tessModule, tessEvalModule, geometryModule);

    wirePipeline = vulkanCreatePipeline(vulkanHandles, presentationEngineInfo,
                                        vkPipelineLayout, renderPass, VK_POLYGON_MODE_LINE, vertModule, fragModule, tessModule, tessEvalModule, geometryModule);
    activePipeline = shadedPipeline;
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex, 0,
                     &graphicsQueue);
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.presentationFamilyIndex, 0,
                     &presentationQueue);
    vkGetDeviceQueue(vulkanHandles.device, physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex, 0,
                     &transferQueue);

    CommandBufferStructure transferStructure{};
    transferStructure.commandBuffer = CommandBufferUtils::vulkanCreateCommandBuffers(vulkanHandles, vkTransferPool,
                                                                                     1)[0];
    transferStructure.bufferAvaibleFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);
    transferStructure.queue = transferQueue;
    transferStructure.queueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.transferFamilyIndex;
    CommandBufferStructure graphicsStructure{};
    graphicsStructure.commandBuffer = CommandBufferUtils::vulkanCreateCommandBuffers(vulkanHandles, vkGraphicsPool,
                                                                                     1)[0];
    graphicsStructure.bufferAvaibleFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);
    graphicsStructure.queue = graphicsQueue;
    graphicsStructure.queueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.graphicsFamilyIndex;

    VkRect2D viewRect{};
    viewRect.extent = presentationEngineInfo.extents;
    viewRect.offset = {0, 0};
    VkClearValue colorClearValue = {1.0, 0.0, 0.0, 1.0};
    VkClearValue depthClearValue = {1.0, 0.0};
    VkFence vkFence = vulkanCreateFence(vulkanHandles, VK_FENCE_CREATE_SIGNALED_BIT);

    Bitmap *image1 = new Bitmap(FileLoader::getPath("Resources/heightmap.bmp"));
    Texture2D texture1 = createTexture2D(vulkanHandles, physicalDeviceInfo, image1->originalBitmapArray,
                                         {(uint32_t) image1->width, (uint32_t) image1->height},
                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer stagingBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo, image1->width * image1->height * 4 * 4,
                                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vulkanMapMemoryWithFlush(vulkanHandles, stagingBuffer, image1->originalBitmapArray);

    copyBufferTextureHostDevice(vulkanHandles, transferStructure, stagingBuffer, texture1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, transferStructure.queueFamilyIndex);
    transitionImageInPipeline(vulkanHandles, graphicsStructure, texture1, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


    VkDescriptorSet textureDescriptorSet = vulkanAllocateDescriptorSet(vulkanHandles, descriptorPool, vkDescriptorSetLayout0);

    VkDescriptorImageInfo vkDescriptorImageInfo{};
    vkDescriptorImageInfo.imageView = texture1.imageView;
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.sampler = texture1.sampler;

    VkWriteDescriptorSet vkWriteDescriptorSet = vulkanGetWriteDescriptorSet(vulkanHandles, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureDescriptorSet, 0, nullptr, &vkDescriptorImageInfo);
    Buffer lightInformationBuffer = allocateExclusiveBuffer(vulkanHandles, physicalDeviceInfo, sizeof(LightInformation), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkDescriptorBufferInfo lightInformationBufferInfo{};
    lightInformationBufferInfo.buffer = lightInformationBuffer.buffer;
    lightInformationBufferInfo.range = lightInformationBuffer.size;
    lightInformationBufferInfo.offset = 0;

    VkWriteDescriptorSet writeLightInformation = vulkanGetWriteDescriptorSet(vulkanHandles, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, textureDescriptorSet, 1, &lightInformationBufferInfo, nullptr);

    std::vector<VkWriteDescriptorSet> descriptorWriteInfo = {vkWriteDescriptorSet, writeLightInformation};

    vkUpdateDescriptorSets(vulkanHandles.device, descriptorWriteInfo.size(), descriptorWriteInfo.data(), 0, nullptr);

    MVP mvp{};
    mvp.model = glm::mat4(1);
    mvp.projetion = glm::perspective(45.0, 1.0, 0.001, 1000.0);
    camera.positionCameraCenter();


    terrainPatches = buildTerrainPatches(vulkanHandles, physicalDeviceInfo, transferStructure, 2, 2,
                                         glm::vec3(2, 2, 2), glm::vec3(1, -3, 1), vkDescriptorSetLayout2, vkDescriptorSetLayout1, descriptorPool);


    int renderFramesAmount = 2;
    std::vector<RenderFrame> renderFrames(renderFramesAmount);
    for (int i = 0; i < renderFramesAmount; ++i) {
        renderFrames[i] = createRenderFrame(vulkanHandles, vkGraphicsPool);
    }
    float frameNumber = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float flash = std::abs(std::sin(frameNumber / 500.f));
        colorClearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

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
                                                              presentationEngineInfo.extents.height, renderPass,
                                                              {swapchainReferences.imageViews[imageIndex], depthMapImageView});

            std::vector<VkClearValue> clearValues = {colorClearValue, depthClearValue};
            VkRenderPassBeginInfo vkRenderPassBeginInfo{};
            vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            vkRenderPassBeginInfo.renderPass = renderPass;
            vkRenderPassBeginInfo.framebuffer = renderFrame.frameBuffer;
            vkRenderPassBeginInfo.renderArea = viewRect;
            vkRenderPassBeginInfo.clearValueCount = clearValues.size();
            vkRenderPassBeginInfo.pClearValues = clearValues.data();


            mvp.view = glm::lookAt(camera.eye, camera.center, camera.up);
            lightInformation.cameraPosition = camera.center;
            glm::mat4 model = glm::mat4(1);
            model = glm::rotate(model, angle, glm::vec3(0, 0, -1));
            angle += 0.001;
            lightInformation.position = model * glm::vec4(0, 10, 0, 1);
            vulkanMapMemoryWithFlush(vulkanHandles, lightInformationBuffer, &lightInformation);

            for (int j = 0; j < terrainPatches.size(); ++j) {
                vulkanMapMemoryWithFlush(vulkanHandles, terrainPatches[j].tessInfoUniform, &terrainPatches[j].tessInfo);
            }

            CommandBufferUtils::vulkanBeginCommandBuffer(vulkanHandles, renderFrame.commandBuffer, 0);
            {
                vkCmdBeginRenderPass(renderFrame.commandBuffer, &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline);
                VkDeviceSize offset = 0;

                vkCmdBindDescriptorSets(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0,
                                        1,
                                        &textureDescriptorSet, 0,
                                        nullptr);

                for (int j = 0; j < terrainPatches.size(); ++j) {
                    terrainPatches[j].mvp.view = mvp.view;
                    terrainPatches[j].mvp.projetion = mvp.projetion;
//                    terrainPatches[j].tessInfo.tessLevelInner = globalInnerTess;
                    terrainPatches[j].tessInfo.tessLevelOuter = glm::vec3(globalOuterTess);
                    vulkanMapMemoryWithFlush(vulkanHandles, terrainPatches[j].mvpUniform, &terrainPatches[j].mvp);
                    vulkanMapMemoryWithFlush(vulkanHandles, terrainPatches[j].tessInfoUniform, &terrainPatches[j].tessInfo);

                    vkCmdBindVertexBuffers(renderFrame.commandBuffer, 0, 1, &terrainPatches[j].vertexBuffer.buffer, &offset);
                    vkCmdBindIndexBuffer(renderFrame.commandBuffer, terrainPatches[j].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdBindDescriptorSets(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 2,
                                            1,
                                            &terrainPatches[j].tessInfoDescriptorSet, 0,
                                            nullptr);
                    vkCmdBindDescriptorSets(renderFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 1,
                                            1,
                                            &terrainPatches[j].mvpDescriptorSet, 0,
                                            nullptr);
                    vkCmdDrawIndexed(renderFrame.commandBuffer, terrainPatches[j].indices.size(), 1, 0, 0, 1);
                }
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
