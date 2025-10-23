#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanSwapchain.h"
#include "utils/Logger.h"
#include <android/log.h>

#define LOG_TAG "VulkanPipeline"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VulkanPipeline::VulkanPipeline()
    : m_initialized(false)
    , m_renderer(nullptr)
    , m_pipeline(VK_NULL_HANDLE)
    , m_layout(VK_NULL_HANDLE)
{
}

VulkanPipeline::~VulkanPipeline() {
    shutdown();
}

bool VulkanPipeline::initialize(VulkanRenderer* renderer) {
    LOGI("Initializing Vulkan pipeline...");
    
    m_renderer = renderer;
    
    if (!createPipelineLayout()) {
        LOGE("Failed to create pipeline layout");
        return false;
    }
    
    if (!createGraphicsPipeline()) {
        LOGE("Failed to create graphics pipeline");
        return false;
    }
    
    m_initialized = true;
    LOGI("Vulkan pipeline initialized successfully");
    return true;
}

void VulkanPipeline::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down Vulkan pipeline...");
        cleanup();
        m_initialized = false;
        LOGI("Vulkan pipeline shutdown complete");
    }
}

VkPipeline VulkanPipeline::getPipeline() const {
    return m_pipeline;
}

VkPipelineLayout VulkanPipeline::getLayout() const {
    return m_layout;
}

bool VulkanPipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    
    VkResult result = vkCreatePipelineLayout(m_renderer->getDevice(), &pipelineLayoutInfo, nullptr, &m_layout);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create pipeline layout: %d", result);
        return false;
    }
    
    LOGI("Pipeline layout created successfully");
    return true;
}

bool VulkanPipeline::createGraphicsPipeline() {
    // For now, create a simple pipeline without shaders
    // In a real implementation, you'd load and compile shaders
    LOGI("Creating graphics pipeline (simplified)");
    
    // Create a basic pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 0; // No shader stages for now
    pipelineInfo.pStages = nullptr;
    
    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    
    // Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 800.0f; // Will be updated dynamically
    viewport.height = 600.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {800, 600};
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    pipelineInfo.pViewportState = &viewportState;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizer;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &multisampling;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    pipelineInfo.pColorBlendState = &colorBlending;
    
    // Dynamic state
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
    pipelineInfo.pDynamicState = &dynamicState;
    
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = m_renderer->m_swapchain->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    VkResult result = vkCreateGraphicsPipelines(m_renderer->getDevice(), VK_NULL_HANDLE, 1, 
                                              &pipelineInfo, nullptr, &m_pipeline);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create graphics pipeline: %d", result);
        return false;
    }
    
    LOGI("Graphics pipeline created successfully");
    return true;
}

void VulkanPipeline::cleanup() {
    if (m_renderer && m_renderer->getDevice() != VK_NULL_HANDLE) {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_renderer->getDevice(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }
        
        if (m_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_renderer->getDevice(), m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
    }
}
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 800.0f; // Will be updated dynamically
    viewport.height = 600.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {800, 600};
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // Dynamic state
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
    
    // Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = m_renderer->m_swapchain->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    VkResult result = vkCreateGraphicsPipelines(m_renderer->getDevice(), VK_NULL_HANDLE, 1, 
                                              &pipelineInfo, nullptr, &m_pipeline);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create graphics pipeline: %d", result);
        return false;
    }
    
    // Clean up shader modules
    vkDestroyShaderModule(m_renderer->getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(m_renderer->getDevice(), vertShaderModule, nullptr);
    
    LOGI("Graphics pipeline created successfully");
    return true;
}

VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(m_renderer->getDevice(), &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create shader module: %d", result);
        return VK_NULL_HANDLE;
    }
    
    return shaderModule;
}

void VulkanPipeline::cleanup() {
    if (m_renderer && m_renderer->getDevice() != VK_NULL_HANDLE) {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_renderer->getDevice(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }
        
        if (m_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_renderer->getDevice(), m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
    }
}

// Simple vertex shader
const std::vector<char> VulkanPipeline::vertShaderCode = {
    // This would be compiled SPIR-V bytecode in a real implementation
    // For now, this is a placeholder
};

// Simple fragment shader
const std::vector<char> VulkanPipeline::fragShaderCode = {
    // This would be compiled SPIR-V bytecode in a real implementation
    // For now, this is a placeholder
};