#pragma once
#include <android/native_window.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class Game;

struct Vertex2D {
    float pos[2];
    float color[3];
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init(ANativeWindow* window);
    void shutdown();
    void resize();

    void beginFrame();
    void draw(const Game& game);
    void endFrame();

    bool isInitialized() const { return initialized; }

    uint32_t getRenderWidth() const { return surfaceWidth; }
    uint32_t getRenderHeight() const { return surfaceHeight; }

private:
    void createInstance();
    void pickPhysicalDevice();
    void createDevice();
    void createSurface(ANativeWindow* window);
    void createSwapchain();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPoolAndBuffers();
    void createSyncObjects();
    void createPipeline();

    void destroySwapchainObjects();

private:
    bool initialized = false;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = 0;
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent{};

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    uint32_t currentImageIndex = 0;

    uint32_t surfaceWidth = 0;
    uint32_t surfaceHeight = 0;
};
