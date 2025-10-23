#include "VulkanCommandBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanSwapchain.h"
#include "utils/Logger.h"
#include <android/log.h>

#define LOG_TAG "VulkanCommandBuffer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VulkanCommandBuffer::VulkanCommandBuffer()
    : m_initialized(false)
    , m_renderer(nullptr)
    , m_currentImageIndex(0)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    shutdown();
}

bool VulkanCommandBuffer::initialize(VulkanRenderer* renderer) {
    LOGI("Initializing Vulkan command buffer...");
    
    m_renderer = renderer;
    
    if (!createCommandBuffers()) {
        LOGE("Failed to create command buffers");
        return false;
    }
    
    m_initialized = true;
    LOGI("Vulkan command buffer initialized successfully");
    return true;
}

void VulkanCommandBuffer::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down Vulkan command buffer...");
        cleanup();
        m_initialized = false;
        LOGI("Vulkan command buffer shutdown complete");
    }
}

void VulkanCommandBuffer::beginFrame(uint32_t imageIndex) {
    if (!m_initialized) {
        return;
    }
    
    m_currentImageIndex = imageIndex;
    VkCommandBuffer commandBuffer = m_commandBuffers[imageIndex];
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOGE("Failed to begin recording command buffer");
        return;
    }
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderer->m_swapchain->getRenderPass();
    renderPassInfo.framebuffer = m_renderer->m_swapchain->getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_renderer->m_swapchain->getExtent();
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderer->m_pipeline->getPipeline());
    
    // Set viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_renderer->m_swapchain->getExtent().width);
    viewport.height = static_cast<float>(m_renderer->m_swapchain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    // Set scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_renderer->m_swapchain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::endFrame() {
    if (!m_initialized) {
        return;
    }
    
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];
    
    // End render pass
    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        LOGE("Failed to record command buffer");
        return;
    }
}

void VulkanCommandBuffer::drawTriangle() {
    if (!m_initialized) {
        return;
    }
    
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];
    
    // Draw a simple triangle
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void VulkanCommandBuffer::drawCube() {
    if (!m_initialized) {
        return;
    }
    
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];
    
    // Draw a cube (simplified - 12 triangles)
    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

void VulkanCommandBuffer::drawSphere(float radius) {
    if (!m_initialized) {
        return;
    }
    
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];
    
    // Draw a sphere (simplified - placeholder)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void VulkanCommandBuffer::drawCylinder() {
    if (!m_initialized) {
        return;
    }
    
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];
    
    // Draw a cylinder (simplified - placeholder)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

VkCommandBuffer VulkanCommandBuffer::getCommandBuffer() const {
    if (m_initialized && m_currentImageIndex < m_commandBuffers.size()) {
        return m_commandBuffers[m_currentImageIndex];
    }
    return VK_NULL_HANDLE;
}

uint32_t VulkanCommandBuffer::getCurrentImageIndex() const {
    return m_currentImageIndex;
}

bool VulkanCommandBuffer::createCommandBuffers() {
    uint32_t imageCount = m_renderer->m_swapchain->getImageViews().size();
    m_commandBuffers.resize(imageCount);
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_renderer->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    
    VkResult result = vkAllocateCommandBuffers(m_renderer->getDevice(), &allocInfo, m_commandBuffers.data());
    if (result != VK_SUCCESS) {
        LOGE("Failed to allocate command buffers: %d", result);
        return false;
    }
    
    LOGI("Created %zu command buffers", m_commandBuffers.size());
    return true;
}

void VulkanCommandBuffer::cleanup() {
    if (m_renderer && m_renderer->getDevice() != VK_NULL_HANDLE && !m_commandBuffers.empty()) {
        vkFreeCommandBuffers(m_renderer->getDevice(), m_renderer->getCommandPool(), 
                           static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }
}