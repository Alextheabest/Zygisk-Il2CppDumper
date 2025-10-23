#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanRenderer;

class VulkanCommandBuffer {
public:
    VulkanCommandBuffer();
    ~VulkanCommandBuffer();
    
    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    void beginFrame(uint32_t imageIndex);
    void endFrame();
    
    // Draw calls
    void drawTriangle();
    void drawCube();
    void drawSphere(float radius);
    void drawCylinder();
    
    VkCommandBuffer getCommandBuffer() const;
    uint32_t getCurrentImageIndex() const;

private:
    bool m_initialized;
    VulkanRenderer* m_renderer;
    std::vector<VkCommandBuffer> m_commandBuffers;
    uint32_t m_currentImageIndex;
    
    bool createCommandBuffers();
    void cleanup();
};