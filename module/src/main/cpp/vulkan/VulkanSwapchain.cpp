#include "VulkanSwapchain.h"
#include "VulkanRenderer.h"
#include "utils/Logger.h"
#include <android/log.h>

#define LOG_TAG "VulkanSwapchain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VulkanSwapchain::VulkanSwapchain()
    : m_initialized(false)
    , m_renderer(nullptr)
    , m_window(nullptr)
    , m_swapchain(VK_NULL_HANDLE)
    , m_imageFormat(VK_FORMAT_UNDEFINED)
    , m_renderPass(VK_NULL_HANDLE)
{
}

VulkanSwapchain::~VulkanSwapchain() {
    shutdown();
}

bool VulkanSwapchain::initialize(VulkanRenderer* renderer, ANativeWindow* window) {
    LOGI("Initializing Vulkan swapchain...");
    
    m_renderer = renderer;
    m_window = window;
    
    if (!createSwapchain()) {
        LOGE("Failed to create swapchain");
        return false;
    }
    
    if (!createImageViews()) {
        LOGE("Failed to create image views");
        return false;
    }
    
    if (!createRenderPass()) {
        LOGE("Failed to create render pass");
        return false;
    }
    
    if (!createFramebuffers()) {
        LOGE("Failed to create framebuffers");
        return false;
    }
    
    m_initialized = true;
    LOGI("Vulkan swapchain initialized successfully");
    return true;
}

void VulkanSwapchain::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down Vulkan swapchain...");
        cleanup();
        m_initialized = false;
        LOGI("Vulkan swapchain shutdown complete");
    }
}

void VulkanSwapchain::resize(int width, int height) {
    if (m_initialized) {
        LOGI("Resizing swapchain to %dx%d", width, height);
        // In a real implementation, you'd recreate the swapchain here
    }
}

VkSwapchainKHR VulkanSwapchain::getSwapchain() const {
    return m_swapchain;
}

VkFormat VulkanSwapchain::getImageFormat() const {
    return m_imageFormat;
}

VkExtent2D VulkanSwapchain::getExtent() const {
    return m_extent;
}

const std::vector<VkImageView>& VulkanSwapchain::getImageViews() const {
    return m_imageViews;
}

VkRenderPass VulkanSwapchain::getRenderPass() const {
    return m_renderPass;
}

const std::vector<VkFramebuffer>& VulkanSwapchain::getFramebuffers() const {
    return m_framebuffers;
}

bool VulkanSwapchain::createSwapchain() {
    // Get surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderer->getPhysicalDevice(), 
                                            m_renderer->getSurface(), &capabilities);
    
    // Get surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->getPhysicalDevice(), 
                                       m_renderer->getSurface(), &formatCount, nullptr);
    
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->getPhysicalDevice(), 
                                       m_renderer->getSurface(), &formatCount, formats.data());
    
    // Choose surface format
    m_imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            m_imageFormat = format.format;
            break;
        }
    }
    
    // Choose extent
    if (capabilities.currentExtent.width != UINT32_MAX) {
        m_extent = capabilities.currentExtent;
    } else {
        int width = ANativeWindow_getWidth(m_window);
        int height = ANativeWindow_getHeight(m_window);
        
        m_extent.width = static_cast<uint32_t>(width);
        m_extent.height = static_cast<uint32_t>(height);
        
        m_extent.width = std::clamp(m_extent.width, capabilities.minImageExtent.width, 
                                   capabilities.maxImageExtent.width);
        m_extent.height = std::clamp(m_extent.height, capabilities.minImageExtent.height, 
                                    capabilities.maxImageExtent.height);
    }
    
    // Create swapchain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_renderer->getSurface();
    createInfo.minImageCount = capabilities.minImageCount + 1;
    createInfo.imageFormat = m_imageFormat;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    VkResult result = vkCreateSwapchainKHR(m_renderer->getDevice(), &createInfo, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create swapchain: %d", result);
        return false;
    }
    
    // Get swapchain images
    vkGetSwapchainImagesKHR(m_renderer->getDevice(), m_swapchain, &formatCount, nullptr);
    m_images.resize(formatCount);
    vkGetSwapchainImagesKHR(m_renderer->getDevice(), m_swapchain, &formatCount, m_images.data());
    
    LOGI("Swapchain created with %d images", formatCount);
    return true;
}

bool VulkanSwapchain::createImageViews() {
    m_imageViews.resize(m_images.size());
    
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        VkResult result = vkCreateImageView(m_renderer->getDevice(), &createInfo, nullptr, &m_imageViews[i]);
        if (result != VK_SUCCESS) {
            LOGE("Failed to create image view %zu: %d", i, result);
            return false;
        }
    }
    
    LOGI("Created %zu image views", m_imageViews.size());
    return true;
}

bool VulkanSwapchain::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    VkResult result = vkCreateRenderPass(m_renderer->getDevice(), &renderPassInfo, nullptr, &m_renderPass);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create render pass: %d", result);
        return false;
    }
    
    LOGI("Render pass created successfully");
    return true;
}

bool VulkanSwapchain::createFramebuffers() {
    m_framebuffers.resize(m_imageViews.size());
    
    for (size_t i = 0; i < m_imageViews.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_imageViews[i];
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;
        
        VkResult result = vkCreateFramebuffer(m_renderer->getDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]);
        if (result != VK_SUCCESS) {
            LOGE("Failed to create framebuffer %zu: %d", i, result);
            return false;
        }
    }
    
    LOGI("Created %zu framebuffers", m_framebuffers.size());
    return true;
}

void VulkanSwapchain::cleanup() {
    if (m_renderer && m_renderer->getDevice() != VK_NULL_HANDLE) {
        for (auto framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_renderer->getDevice(), framebuffer, nullptr);
        }
        m_framebuffers.clear();
        
        if (m_renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_renderer->getDevice(), m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }
        
        for (auto imageView : m_imageViews) {
            vkDestroyImageView(m_renderer->getDevice(), imageView, nullptr);
        }
        m_imageViews.clear();
        
        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_renderer->getDevice(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }
}