#pragma once

#include <vulkan/vulkan.h>
#include <android/native_window.h>
#include <vector>

class VulkanRenderer;

class VulkanSwapchain {
public:
    VulkanSwapchain();
    ~VulkanSwapchain();
    
    bool initialize(VulkanRenderer* renderer, ANativeWindow* window);
    void shutdown();
    void resize(int width, int height);
    
    VkSwapchainKHR getSwapchain() const;
    VkFormat getImageFormat() const;
    VkExtent2D getExtent() const;
    const std::vector<VkImageView>& getImageViews() const;
    VkRenderPass getRenderPass() const;
    const std::vector<VkFramebuffer>& getFramebuffers() const;

private:
    bool m_initialized;
    VulkanRenderer* m_renderer;
    ANativeWindow* m_window;
    
    VkSwapchainKHR m_swapchain;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
    
    bool createSwapchain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    void cleanup();
};