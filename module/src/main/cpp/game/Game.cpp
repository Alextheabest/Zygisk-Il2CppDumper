#include "Game.h"
#include "PoolGame.h"
#include "vulkan/VulkanRenderer.h"
#include "utils/Logger.h"
#include <android/log.h>

#define LOG_TAG "Game"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Game::Game() 
    : m_initialized(false)
    , m_paused(false)
    , m_width(0)
    , m_height(0)
    , m_window(nullptr)
    , m_assetManager(nullptr)
{
}

Game::~Game() {
    shutdown();
}

bool Game::initialize(ANativeWindow* window, AAssetManager* assetManager) {
    LOGI("Initializing game...");
    
    m_window = window;
    m_assetManager = assetManager;
    
    // Get window dimensions
    m_width = ANativeWindow_getWidth(window);
    m_height = ANativeWindow_getHeight(window);
    
    LOGI("Window size: %dx%d", m_width, m_height);
    
    // Initialize Vulkan renderer
    m_renderer = std::make_unique<VulkanRenderer>();
    if (!m_renderer->initialize(window, assetManager)) {
        LOGE("Failed to initialize Vulkan renderer");
        return false;
    }
    
    // Initialize pool game
    m_poolGame = std::make_unique<PoolGame>();
    if (!m_poolGame->initialize(m_renderer.get())) {
        LOGE("Failed to initialize pool game");
        return false;
    }
    
    m_initialized = true;
    LOGI("Game initialized successfully");
    return true;
}

void Game::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down game...");
        
        if (m_poolGame) {
            m_poolGame->shutdown();
            m_poolGame.reset();
        }
        
        if (m_renderer) {
            m_renderer->shutdown();
            m_renderer.reset();
        }
        
        m_initialized = false;
        LOGI("Game shutdown complete");
    }
}

void Game::update() {
    if (!m_initialized || m_paused) {
        return;
    }
    
    updateInput();
    updateGameLogic();
    
    if (m_poolGame) {
        m_poolGame->update();
    }
}

void Game::render() {
    if (!m_initialized || !m_renderer) {
        return;
    }
    
    m_renderer->beginFrame();
    
    if (m_poolGame) {
        m_poolGame->render(m_renderer.get());
    }
    
    m_renderer->endFrame();
}

void Game::pause() {
    LOGI("Game paused");
    m_paused = true;
}

void Game::resume() {
    LOGI("Game resumed");
    m_paused = false;
}

void Game::resize(int width, int height) {
    LOGI("Resizing to %dx%d", width, height);
    m_width = width;
    m_height = height;
    
    if (m_renderer) {
        m_renderer->resize(width, height);
    }
    
    if (m_poolGame) {
        m_poolGame->resize(width, height);
    }
}

void Game::onTouch(float x, float y, int action) {
    if (m_poolGame) {
        m_poolGame->onTouch(x, y, action);
    }
}

void Game::updateInput() {
    // Handle input updates
}

void Game::updateGameLogic() {
    // Handle game logic updates
}