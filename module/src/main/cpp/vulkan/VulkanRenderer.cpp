#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "utils/Logger.h"
#include <android/log.h>
#include <cstring>

#define LOG_TAG "VulkanRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VulkanRenderer::VulkanRenderer()
    : m_initialized(false)
    , m_window(nullptr)
    , m_assetManager(nullptr)
    , m_instance(VK_NULL_HANDLE)
    , m_surface(VK_NULL_HANDLE)
    , m_physicalDevice(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE)
    , m_presentQueue(VK_NULL_HANDLE)
    , m_commandPool(VK_NULL_HANDLE)
    , m_imageAvailableSemaphore(VK_NULL_HANDLE)
    , m_renderFinishedSemaphore(VK_NULL_HANDLE)
    , m_inFlightFence(VK_NULL_HANDLE)
{
    // Initialize matrices to identity
    memset(m_viewMatrix, 0, sizeof(m_viewMatrix));
    memset(m_projectionMatrix, 0, sizeof(m_projectionMatrix));
    memset(m_modelMatrix, 0, sizeof(m_modelMatrix));
    
    for (int i = 0; i < 4; ++i) {
        m_viewMatrix[i * 4 + i] = 1.0f;
        m_projectionMatrix[i * 4 + i] = 1.0f;
        m_modelMatrix[i * 4 + i] = 1.0f;
    }
    
    // Initialize color to white
    m_color[0] = 1.0f;
    m_color[1] = 1.0f;
    m_color[2] = 1.0f;
    m_color[3] = 1.0f;
}

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::initialize(ANativeWindow* window, AAssetManager* assetManager) {
    LOGI("Initializing Vulkan renderer...");
    
    m_window = window;
    m_assetManager = assetManager;
    
    // Create Vulkan instance
    if (!createInstance()) {
        LOGE("Failed to create Vulkan instance");
        return false;
    }
    
    // Create surface
    if (!createSurface()) {
        LOGE("Failed to create surface");
        return false;
    }
    
    // Select physical device
    if (!selectPhysicalDevice()) {
        LOGE("Failed to select physical device");
        return false;
    }
    
    // Create logical device
    if (!createDevice()) {
        LOGE("Failed to create logical device");
        return false;
    }
    
    // Create command pool
    if (!createCommandPool()) {
        LOGE("Failed to create command pool");
        return false;
    }
    
    // Create synchronization objects
    if (!createSyncObjects()) {
        LOGE("Failed to create synchronization objects");
        return false;
    }
    
    // Create swapchain
    m_swapchain = std::make_unique<VulkanSwapchain>();
    if (!m_swapchain->initialize(this, m_window)) {
        LOGE("Failed to initialize swapchain");
        return false;
    }
    
    // Create pipeline
    m_pipeline = std::make_unique<VulkanPipeline>();
    if (!m_pipeline->initialize(this)) {
        LOGE("Failed to initialize pipeline");
        return false;
    }
    
    // Create command buffer
    m_commandBuffer = std::make_unique<VulkanCommandBuffer>();
    if (!m_commandBuffer->initialize(this)) {
        LOGE("Failed to initialize command buffer");
        return false;
    }
    
    m_initialized = true;
    LOGI("Vulkan renderer initialized successfully");
    return true;
}

void VulkanRenderer::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down Vulkan renderer...");
        
        vkDeviceWaitIdle(m_device);
        
        if (m_commandBuffer) {
            m_commandBuffer->shutdown();
            m_commandBuffer.reset();
        }
        
        if (m_pipeline) {
            m_pipeline->shutdown();
            m_pipeline.reset();
        }
        
        if (m_swapchain) {
            m_swapchain->shutdown();
            m_swapchain.reset();
        }
        
        cleanup();
        
        m_initialized = false;
        LOGI("Vulkan renderer shutdown complete");
    }
}

void VulkanRenderer::beginFrame() {
    if (!m_initialized) {
        return;
    }
    
    // Wait for previous frame to finish
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);
    
    // Acquire next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain->getSwapchain(), UINT64_MAX,
                                          m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (result != VK_SUCCESS) {
        LOGE("Failed to acquire swap chain image");
        return;
    }
    
    // Begin command buffer
    if (m_commandBuffer) {
        m_commandBuffer->beginFrame(imageIndex);
    }
}

void VulkanRenderer::endFrame() {
    if (!m_initialized || !m_commandBuffer) {
        return;
    }
    
    // End command buffer
    m_commandBuffer->endFrame();
    
    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer->getCommandBuffer();
    
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        LOGE("Failed to submit draw command buffer");
        return;
    }
    
    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {m_swapchain->getSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_commandBuffer->getCurrentImageIndex();
    
    vkQueuePresentKHR(m_presentQueue, &presentInfo);
}

void VulkanRenderer::resize(int width, int height) {
    if (m_swapchain) {
        m_swapchain->resize(width, height);
    }
}

void VulkanRenderer::setViewMatrix(const float* matrix) {
    memcpy(m_viewMatrix, matrix, sizeof(m_viewMatrix));
}

void VulkanRenderer::setProjectionMatrix(const float* matrix) {
    memcpy(m_projectionMatrix, matrix, sizeof(m_projectionMatrix));
}

void VulkanRenderer::setModelMatrix(const float* matrix) {
    memcpy(m_modelMatrix, matrix, sizeof(m_modelMatrix));
}

void VulkanRenderer::setColor(float r, float g, float b, float a) {
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;
}

void VulkanRenderer::drawTriangle() {
    if (m_commandBuffer) {
        m_commandBuffer->drawTriangle();
    }
}

void VulkanRenderer::drawCube() {
    if (m_commandBuffer) {
        m_commandBuffer->drawCube();
    }
}

void VulkanRenderer::drawSphere(float radius) {
    if (m_commandBuffer) {
        m_commandBuffer->drawSphere(radius);
    }
}

void VulkanRenderer::drawCylinder() {
    if (m_commandBuffer) {
        m_commandBuffer->drawCylinder();
    }
}

VkDevice VulkanRenderer::getDevice() const {
    return m_device;
}

VkPhysicalDevice VulkanRenderer::getPhysicalDevice() const {
    return m_physicalDevice;
}

VkQueue VulkanRenderer::getGraphicsQueue() const {
    return m_graphicsQueue;
}

VkCommandPool VulkanRenderer::getCommandPool() const {
    return m_commandPool;
}

VkSurfaceKHR VulkanRenderer::getSurface() const {
    return m_surface;
}

bool VulkanRenderer::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "8-Ball Pool";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    // Enable validation layers in debug builds
    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;
    
    // Enable required extensions
    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
    };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create Vulkan instance: %d", result);
        return false;
    }
    
    LOGI("Vulkan instance created successfully");
    return true;
}

bool VulkanRenderer::createSurface() {
    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = m_window;
    
    VkResult result = vkCreateAndroidSurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create Android surface: %d", result);
        return false;
    }
    
    LOGI("Android surface created successfully");
    return true;
}

bool VulkanRenderer::selectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        LOGE("Failed to find GPUs with Vulkan support");
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    // Select the first suitable device
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        
        LOGI("Found device: %s", deviceProperties.deviceName);
        
        // Check if device supports required features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        
        if (deviceFeatures.geometryShader) {
            m_physicalDevice = device;
            LOGI("Selected physical device: %s", deviceProperties.deviceName);
            return true;
        }
    }
    
    LOGE("Failed to find a suitable GPU");
    return false;
}

bool VulkanRenderer::createDevice() {
    // Find queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    int graphicsFamily = -1;
    int presentFamily = -1;
    
    for (int i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport) {
            presentFamily = i;
        }
    }
    
    if (graphicsFamily == -1 || presentFamily == -1) {
        LOGE("Failed to find suitable queue families");
        return false;
    }
    
    // Create device
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.geometryShader = VK_TRUE;
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    
    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = extensions;
    
    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create logical device: %d", result);
        return false;
    }
    
    vkGetDeviceQueue(m_device, graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, presentFamily, 0, &m_presentQueue);
    
    LOGI("Logical device created successfully");
    return true;
}

bool VulkanRenderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = 0; // Graphics queue family
    
    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    if (result != VK_SUCCESS) {
        LOGE("Failed to create command pool: %d", result);
        return false;
    }
    
    LOGI("Command pool created successfully");
    return true;
}

bool VulkanRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
        LOGE("Failed to create synchronization objects");
        return false;
    }
    
    LOGI("Synchronization objects created successfully");
    return true;
}

void VulkanRenderer::cleanup() {
    if (m_inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_inFlightFence, nullptr);
        m_inFlightFence = VK_NULL_HANDLE;
    }
    
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }
    
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
    
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}