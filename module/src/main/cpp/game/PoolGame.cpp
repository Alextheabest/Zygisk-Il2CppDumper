#include "PoolGame.h"
#include "Table.h"
#include "Ball.h"
#include "Cue.h"
#include "renderer/Camera.h"
#include "utils/Logger.h"
#include <android/log.h>

#define LOG_TAG "PoolGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

PoolGame::PoolGame()
    : m_initialized(false)
    , m_width(0)
    , m_height(0)
    , m_gameState(GameState::PLACING_CUE)
    , m_currentPlayer(1)
    , m_gameStarted(false)
{
}

PoolGame::~PoolGame() {
    shutdown();
}

bool PoolGame::initialize(VulkanRenderer* renderer) {
    LOGI("Initializing pool game...");
    
    if (!renderer) {
        LOGE("Renderer is null");
        return false;
    }
    
    // Initialize table
    m_table = std::make_unique<Table>();
    if (!m_table->initialize(renderer)) {
        LOGE("Failed to initialize table");
        return false;
    }
    
    // Initialize camera
    m_camera = std::make_unique<Camera>();
    m_camera->setPosition(0.0f, 2.0f, 5.0f);
    m_camera->lookAt(0.0f, 0.0f, 0.0f);
    
    // Initialize cue
    m_cue = std::make_unique<Cue>();
    if (!m_cue->initialize(renderer)) {
        LOGE("Failed to initialize cue");
        return false;
    }
    
    // Initialize balls
    initializeBalls();
    
    m_initialized = true;
    LOGI("Pool game initialized successfully");
    return true;
}

void PoolGame::shutdown() {
    if (m_initialized) {
        LOGI("Shutting down pool game...");
        
        if (m_cue) {
            m_cue->shutdown();
            m_cue.reset();
        }
        
        m_balls.clear();
        
        if (m_table) {
            m_table->shutdown();
            m_table.reset();
        }
        
        if (m_camera) {
            m_camera.reset();
        }
        
        m_initialized = false;
        LOGI("Pool game shutdown complete");
    }
}

void PoolGame::update() {
    if (!m_initialized) {
        return;
    }
    
    // Update physics
    updatePhysics(1.0f / 60.0f); // Assuming 60 FPS
    
    // Check collisions
    checkCollisions();
    
    // Update game state
    updateGameState();
    
    // Update table
    if (m_table) {
        m_table->update();
    }
    
    // Update balls
    for (auto& ball : m_balls) {
        if (ball) {
            ball->update();
        }
    }
    
    // Update cue
    if (m_cue) {
        m_cue->update();
    }
}

void PoolGame::render(VulkanRenderer* renderer) {
    if (!m_initialized || !renderer) {
        return;
    }
    
    // Set camera matrices
    if (m_camera) {
        renderer->setViewMatrix(m_camera->getViewMatrix());
        renderer->setProjectionMatrix(m_camera->getProjectionMatrix());
    }
    
    // Render table
    if (m_table) {
        m_table->render(renderer);
    }
    
    // Render balls
    for (auto& ball : m_balls) {
        if (ball) {
            ball->render(renderer);
        }
    }
    
    // Render cue
    if (m_cue) {
        m_cue->render(renderer);
    }
}

void PoolGame::resize(int width, int height) {
    m_width = width;
    m_height = height;
    
    if (m_camera) {
        m_camera->setAspectRatio((float)width / (float)height);
    }
}

void PoolGame::onTouch(float x, float y, int action) {
    // Convert screen coordinates to world coordinates
    // This is a simplified version - in a real implementation,
    // you'd need proper screen-to-world coordinate conversion
    
    if (m_cue) {
        m_cue->onTouch(x, y, action);
    }
}

void PoolGame::initializeBalls() {
    LOGI("Initializing balls...");
    
    // Create cue ball (white ball)
    auto cueBall = std::make_unique<Ball>();
    cueBall->setType(Ball::Type::CUE);
    cueBall->setPosition(0.0f, 0.0f, -2.0f);
    m_balls.push_back(std::move(cueBall));
    
    // Create numbered balls (1-15)
    for (int i = 1; i <= 15; ++i) {
        auto ball = std::make_unique<Ball>();
        ball->setType(static_cast<Ball::Type>(i));
        ball->setPosition(0.0f, 0.0f, 2.0f + i * 0.1f);
        m_balls.push_back(std::move(ball));
    }
    
    LOGI("Initialized %zu balls", m_balls.size());
}

void PoolGame::updatePhysics(float deltaTime) {
    // Update ball physics
    for (auto& ball : m_balls) {
        if (ball) {
            ball->updatePhysics(deltaTime);
        }
    }
}

void PoolGame::checkCollisions() {
    // Ball-to-ball collisions
    for (size_t i = 0; i < m_balls.size(); ++i) {
        for (size_t j = i + 1; j < m_balls.size(); ++j) {
            if (m_balls[i] && m_balls[j]) {
                // Check collision between balls[i] and balls[j]
                // This would involve checking distance and applying collision response
            }
        }
    }
    
    // Ball-to-table collisions
    for (auto& ball : m_balls) {
        if (ball && m_table) {
            // Check collision with table boundaries and pockets
            m_table->checkBallCollision(ball.get());
        }
    }
}

void PoolGame::updateGameState() {
    // Check if all balls are stationary
    bool allStationary = true;
    for (auto& ball : m_balls) {
        if (ball && ball->isMoving()) {
            allStationary = false;
            break;
        }
    }
    
    if (allStationary && m_gameState == GameState::BALLS_MOVING) {
        m_gameState = GameState::PLACING_CUE;
    }
}

void PoolGame::resetGame() {
    LOGI("Resetting game...");
    
    // Reset ball positions
    initializeBalls();
    
    // Reset game state
    m_gameState = GameState::PLACING_CUE;
    m_currentPlayer = 1;
    m_gameStarted = false;
}