#pragma once

#include <vulkan/vulkan.h>

class VulkanRenderer;

class VulkanPipeline {
public:
    VulkanPipeline();
    ~VulkanPipeline();
    
    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    VkPipeline getPipeline() const;
    VkPipelineLayout getLayout() const;

private:
    bool m_initialized;
    VulkanRenderer* m_renderer;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    
    bool createPipelineLayout();
    bool createGraphicsPipeline();
    void cleanup();
};