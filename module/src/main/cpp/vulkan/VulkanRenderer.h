#pragma once

#include <vulkan/vulkan.h>
#include <android/native_window.h>
#include <android/asset_manager.h>
#include <vector>
#include <memory>

class VulkanDevice;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanCommandBuffer;

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();
    
    bool initialize(ANativeWindow* window, AAssetManager* assetManager);
    void shutdown();
    
    void beginFrame();
    void endFrame();
    void resize(int width, int height);
    
    // Rendering
    void setViewMatrix(const float* matrix);
    void setProjectionMatrix(const float* matrix);
    void setModelMatrix(const float* matrix);
    void setColor(float r, float g, float b, float a);
    
    // Draw calls
    void drawTriangle();
    void drawCube();
    void drawSphere(float radius);
    void drawCylinder();
    
    // Getters
    VkDevice getDevice() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkQueue getGraphicsQueue() const;
    VkCommandPool getCommandPool() const;
    VkSurfaceKHR getSurface() const;

private:
    bool m_initialized;
    ANativeWindow* m_window;
    AAssetManager* m_assetManager;
    
    // Vulkan core
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkCommandPool m_commandPool;
    
    // Make surface accessible to other classes
    friend class VulkanSwapchain;
    
    // Rendering
    std::unique_ptr<VulkanSwapchain> m_swapchain;
    std::unique_ptr<VulkanPipeline> m_pipeline;
    std::unique_ptr<VulkanCommandBuffer> m_commandBuffer;
    
    // Matrices
    float m_viewMatrix[16];
    float m_projectionMatrix[16];
    float m_modelMatrix[16];
    float m_color[4];
    
    // Frame synchronization
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;
    
    bool createInstance();
    bool createSurface();
    bool selectPhysicalDevice();
    bool createDevice();
    bool createCommandPool();
    bool createSyncObjects();
    
    void cleanup();
};