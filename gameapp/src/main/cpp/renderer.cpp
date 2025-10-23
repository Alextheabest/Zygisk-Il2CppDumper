#include "renderer.h"
#include "game.h"
#include <android/log.h>
#include <android/native_window_jni.h>
#include <cassert>
#include <cstring>
#include <fstream>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Pool", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Pool", __VA_ARGS__)

static std::vector<char> readFile(const char* path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) return {};
    size_t size = (size_t) file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

Renderer::Renderer() = default;
Renderer::~Renderer() { shutdown(); }

void Renderer::init(ANativeWindow* window) {
    if (initialized) return;
    createInstance();
    createSurface(window);
    pickPhysicalDevice();
    createDevice();
    createSwapchain();
    createRenderPass();
    createFramebuffers();
    createCommandPoolAndBuffers();
    createSyncObjects();
    createPipeline();
    initialized = true;
}

void Renderer::shutdown() {
    if (!device) return;
    vkDeviceWaitIdle(device);

    destroySwapchainObjects();

    if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
    if (pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    if (commandPool) vkDestroyCommandPool(device, commandPool, nullptr);

    if (imageAvailable) vkDestroySemaphore(device, imageAvailable, nullptr);
    if (renderFinished) vkDestroySemaphore(device, renderFinished, nullptr);
    if (inFlightFence) vkDestroyFence(device, inFlightFence, nullptr);

    if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);

    if (device) vkDestroyDevice(device, nullptr);
    if (instance) vkDestroyInstance(instance, nullptr);

    instance = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    initialized = false;
}

void Renderer::resize() {
    if (!device) return;
    vkDeviceWaitIdle(device);
    destroySwapchainObjects();
    createSwapchain();
    createRenderPass();
    createFramebuffers();
    createCommandPoolAndBuffers();
    createSyncObjects();
    createPipeline();
}

void Renderer::beginFrame() {
    if (!initialized) return;
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &currentImageIndex);
}

void Renderer::draw(const Game& game) {
    if (!initialized) return;

    VkCommandBuffer cmd = commandBuffers[currentImageIndex];

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkClearValue clear{};
    clear.color = { { 0.02f, 0.3f, 0.08f, 1.0f } };

    VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rp.renderPass = renderPass;
    rp.framebuffer = framebuffers[currentImageIndex];
    rp.renderArea.offset = {0, 0};
    rp.renderArea.extent = swapchainExtent;
    rp.clearValueCount = 1;
    rp.pClearValues = &clear;

    vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // Ask game to record draws
    game.recordDrawCommands(cmd);

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}

void Renderer::endFrame() {
    if (!initialized) return;
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailable;
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffers[currentImageIndex];
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished;

    vkQueueSubmit(graphicsQueue, 1, &submit, inFlightFence);

    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain;
    present.pImageIndices = &currentImageIndex;

    vkQueuePresentKHR(graphicsQueue, &present);
}

void Renderer::createInstance() {
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.apiVersion = VK_API_VERSION_1_1;
    app.pApplicationName = "Pool Vulkan";
    app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    const char* exts[] = { "VK_KHR_surface", "VK_KHR_android_surface" };

    VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = 2;
    ci.ppEnabledExtensionNames = exts;

    VkResult r = vkCreateInstance(&ci, nullptr, &instance);
    assert(r == VK_SUCCESS);
}

void Renderer::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    for (auto d : devices) {
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> props(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qCount, props.data());
        for (uint32_t i = 0; i < qCount; ++i) {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface, &presentSupport);
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                physicalDevice = d;
                graphicsQueueFamily = i;
                return;
            }
        }
    }
    assert(physicalDevice != VK_NULL_HANDLE);
}

void Renderer::createDevice() {
    float prio = 1.0f;
    VkDeviceQueueCreateInfo q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    q.queueFamilyIndex = graphicsQueueFamily;
    q.queueCount = 1;
    q.pQueuePriorities = &prio;

    const char* exts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &q;
    ci.enabledExtensionCount = 1;
    ci.ppEnabledExtensionNames = exts;

    VkResult r = vkCreateDevice(physicalDevice, &ci, nullptr, &device);
    assert(r == VK_SUCCESS);
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
}

void Renderer::createSurface(ANativeWindow* window) {
    VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    sci.window = window;
    VkResult r = vkCreateAndroidSurfaceKHR(instance, &sci, nullptr, &surface);
    assert(r == VK_SUCCESS);

    surfaceWidth = ANativeWindow_getWidth(window);
    surfaceHeight = ANativeWindow_getHeight(window);
}

void Renderer::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

    swapchainExtent = caps.currentExtent;
    if (swapchainExtent.width == 0 || swapchainExtent.height == 0) {
        swapchainExtent = {surfaceWidth, surfaceHeight};
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    swapchainFormat = formats[0].format;

    VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    ci.surface = surface;
    ci.minImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && ci.minImageCount > caps.maxImageCount) ci.minImageCount = caps.maxImageCount;
    ci.imageFormat = swapchainFormat;
    ci.imageColorSpace = formats[0].colorSpace;
    ci.imageExtent = swapchainExtent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR; // always available
    ci.clipped = VK_TRUE;

    VkResult r = vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain);
    assert(r == VK_SUCCESS);

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i) {
        VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vi.image = swapchainImages[i];
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = swapchainFormat;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.layerCount = 1;
        VkResult rv = vkCreateImageView(device, &vi, nullptr, &swapchainImageViews[i]);
        assert(rv == VK_SUCCESS);
    }
}

void Renderer::createRenderPass() {
    if (renderPass) vkDestroyRenderPass(device, renderPass, nullptr);

    VkAttachmentDescription color{};
    color.format = swapchainFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rp.attachmentCount = 1;
    rp.pAttachments = &color;
    rp.subpassCount = 1;
    rp.pSubpasses = &subpass;
    rp.dependencyCount = 1;
    rp.pDependencies = &dep;

    VkResult r = vkCreateRenderPass(device, &rp, nullptr, &renderPass);
    assert(r == VK_SUCCESS);
}

void Renderer::createFramebuffers() {
    for (auto fb : framebuffers) {
        if (fb) vkDestroyFramebuffer(device, fb, nullptr);
    }
    framebuffers.clear();
    framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
        VkImageView attachments[] = { swapchainImageViews[i] };
        VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fi.renderPass = renderPass;
        fi.attachmentCount = 1;
        fi.pAttachments = attachments;
        fi.width = swapchainExtent.width;
        fi.height = swapchainExtent.height;
        fi.layers = 1;
        VkResult r = vkCreateFramebuffer(device, &fi, nullptr, &framebuffers[i]);
        assert(r == VK_SUCCESS);
    }
}

void Renderer::createCommandPoolAndBuffers() {
    if (commandPool) vkDestroyCommandPool(device, commandPool, nullptr);

    VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pi.queueFamilyIndex = graphicsQueueFamily;
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult r = vkCreateCommandPool(device, &pi, nullptr, &commandPool);
    assert(r == VK_SUCCESS);

    commandBuffers.resize(framebuffers.size());
    VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    ai.commandPool = commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = (uint32_t)commandBuffers.size();
    r = vkAllocateCommandBuffers(device, &ai, commandBuffers.data());
    assert(r == VK_SUCCESS);
}

void Renderer::createSyncObjects() {
    if (imageAvailable) vkDestroySemaphore(device, imageAvailable, nullptr);
    if (renderFinished) vkDestroySemaphore(device, renderFinished, nullptr);
    if (inFlightFence) vkDestroyFence(device, inFlightFence, nullptr);

    VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(device, &si, nullptr, &imageAvailable);
    vkCreateSemaphore(device, &si, nullptr, &renderFinished);

    VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fi, nullptr, &inFlightFence);
}

void Renderer::createPipeline() {
    if (pipeline) {
        vkDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (pipelineLayout) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }

    // Load precompiled shaders from assets path (packaged by Gradle task)
    // On device, assets are not regular files. For simplicity in this minimal sample, we expect
    // the .spv to be deployed as regular files in the working directory by build system.
    // Fallback to hardcoded default shader binary arrays would be better in production.
    auto vert = readFile("/data/local/tmp/shaders/simple.vert.spv");
    auto frag = readFile("/data/local/tmp/shaders/simple.frag.spv");
    if (vert.empty() || frag.empty()) {
        LOGE("Shaders not found; pipeline creation will likely fail.");
    }

    VkShaderModuleCreateInfo smi{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    smi.codeSize = vert.size();
    smi.pCode = reinterpret_cast<const uint32_t*>(vert.data());
    VkShaderModule vsm = VK_NULL_HANDLE;
    if (smi.codeSize) vkCreateShaderModule(device, &smi, nullptr, &vsm);

    smi.codeSize = frag.size();
    smi.pCode = reinterpret_cast<const uint32_t*>(frag.data());
    VkShaderModule fsm = VK_NULL_HANDLE;
    if (smi.codeSize) vkCreateShaderModule(device, &smi, nullptr, &fsm);

    VkPipelineShaderStageCreateInfo vs{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vs.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs.module = vsm;
    vs.pName = "main";

    VkPipelineShaderStageCreateInfo fs{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fs.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs.module = fsm;
    fs.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vs, fs };

    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(float) * 5; // pos(2) + color(3)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[2]{};
    attrs[0].binding = 0; attrs[0].location = 0; attrs[0].format = VK_FORMAT_R32G32_SFLOAT; attrs[0].offset = 0;
    attrs[1].binding = 0; attrs[1].location = 1; attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[1].offset = sizeof(float)*2;

    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &binding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1; vp.pViewports = &viewport;
    vp.scissorCount = 1; vp.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1; cb.pAttachments = &blend;

    VkPipelineLayoutCreateInfo pl{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    vkCreatePipelineLayout(device, &pl, nullptr, &pipelineLayout);

    VkGraphicsPipelineCreateInfo gp{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gp.stageCount = 2; gp.pStages = stages;
    gp.pVertexInputState = &vi;
    gp.pInputAssemblyState = &ia;
    gp.pViewportState = &vp;
    gp.pRasterizationState = &rs;
    gp.pMultisampleState = &ms;
    gp.pColorBlendState = &cb;
    gp.layout = pipelineLayout;
    gp.renderPass = renderPass;
    gp.subpass = 0;

    VkResult r = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gp, nullptr, &pipeline);
    if (r != VK_SUCCESS) {
        LOGE("Failed to create graphics pipeline (%d). Rendering will be blank.", r);
    }

    if (vsm) vkDestroyShaderModule(device, vsm, nullptr);
    if (fsm) vkDestroyShaderModule(device, fsm, nullptr);
}

void Renderer::destroySwapchainObjects() {
    for (auto fb : framebuffers) if (fb) vkDestroyFramebuffer(device, fb, nullptr);
    framebuffers.clear();

    if (renderPass) { vkDestroyRenderPass(device, renderPass, nullptr); renderPass = VK_NULL_HANDLE; }

    for (auto iv : swapchainImageViews) if (iv) vkDestroyImageView(device, iv, nullptr);
    swapchainImageViews.clear();

    if (swapchain) { vkDestroySwapchainKHR(device, swapchain, nullptr); swapchain = VK_NULL_HANDLE; }
}
