#include "Table.h"
#include "vulkan/VulkanRenderer.h"
#include "utils/Logger.h"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "Table"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Table::Table()
    : m_initialized(false)
    , m_width(8.0f)
    , m_height(4.0f)
    , m_pocketRadius(0.3f)
    , m_leftBound(-4.0f)
    , m_rightBound(4.0f)
    , m_topBound(-2.0f)
    , m_bottomBound(2.0f)
{
}

Table::~Table() {
    shutdown();
}

bool Table::initialize(VulkanRenderer* renderer) {
    if (!renderer) {
        LOGE("Renderer is null");
        return false;
    }
    
    initializePockets();
    
    m_initialized = true;
    LOGI("Table initialized with dimensions %.1fx%.1f", m_width, m_height);
    return true;
}

void Table::shutdown() {
    if (m_initialized) {
        m_initialized = false;
        LOGI("Table shutdown");
    }
}

void Table::update() {
    if (!m_initialized) {
        return;
    }
    
    // Update table state if needed
}

void Table::render(VulkanRenderer* renderer) {
    if (!m_initialized || !renderer) {
        return;
    }
    
    // Set table color (green felt)
    renderer->setColor(0.0f, 0.5f, 0.0f, 1.0f);
    
    // Render table surface
    Matrix4 tableMatrix = Matrix4::translation(Vector3(0.0f, -0.1f, 0.0f)) * 
                         Matrix4::scale(Vector3(m_width, 0.1f, m_height));
    renderer->setModelMatrix(tableMatrix);
    renderer->drawCube();
    
    // Render table rails
    renderer->setColor(0.4f, 0.2f, 0.1f, 1.0f); // Brown wood color
    
    // Left rail
    Matrix4 leftRail = Matrix4::translation(Vector3(m_leftBound - 0.1f, 0.0f, 0.0f)) *
                      Matrix4::scale(Vector3(0.2f, 0.3f, m_height + 0.4f));
    renderer->setModelMatrix(leftRail);
    renderer->drawCube();
    
    // Right rail
    Matrix4 rightRail = Matrix4::translation(Vector3(m_rightBound + 0.1f, 0.0f, 0.0f)) *
                       Matrix4::scale(Vector3(0.2f, 0.3f, m_height + 0.4f));
    renderer->setModelMatrix(rightRail);
    renderer->drawCube();
    
    // Top rail
    Matrix4 topRail = Matrix4::translation(Vector3(0.0f, 0.0f, m_topBound - 0.1f)) *
                     Matrix4::scale(Vector3(m_width + 0.4f, 0.3f, 0.2f));
    renderer->setModelMatrix(topRail);
    renderer->drawCube();
    
    // Bottom rail
    Matrix4 bottomRail = Matrix4::translation(Vector3(0.0f, 0.0f, m_bottomBound + 0.1f)) *
                        Matrix4::scale(Vector3(m_width + 0.4f, 0.3f, 0.2f));
    renderer->setModelMatrix(bottomRail);
    renderer->drawCube();
    
    // Render pockets
    renderer->setColor(0.0f, 0.0f, 0.0f, 1.0f); // Black
    for (int i = 0; i < 6; ++i) {
        Matrix4 pocketMatrix = Matrix4::translation(m_pockets[i]) *
                              Matrix4::scale(m_pocketRadius);
        renderer->setModelMatrix(pocketMatrix);
        renderer->drawSphere(1.0f);
    }
}

void Table::resize(int width, int height) {
    // Update aspect ratio if needed
}

void Table::checkBallCollision(Ball* ball) {
    if (!ball || !m_initialized) {
        return;
    }
    
    // Check pocket collision
    if (isBallInPocket(ball)) {
        handlePocketCollision(ball);
        return;
    }
    
    // Check wall collision
    handleWallCollision(ball);
}

float Table::getWidth() const {
    return m_width;
}

float Table::getHeight() const {
    return m_height;
}

float Table::getPocketRadius() const {
    return m_pocketRadius;
}

void Table::setWidth(float width) {
    m_width = width;
    m_leftBound = -width * 0.5f;
    m_rightBound = width * 0.5f;
    initializePockets();
}

void Table::setHeight(float height) {
    m_height = height;
    m_topBound = -height * 0.5f;
    m_bottomBound = height * 0.5f;
    initializePockets();
}

void Table::setPocketRadius(float radius) {
    m_pocketRadius = radius;
}

void Table::initializePockets() {
    // Corner pockets
    m_pockets[0] = Vector3(m_leftBound, 0.0f, m_topBound);    // Top-left
    m_pockets[1] = Vector3(m_rightBound, 0.0f, m_topBound);   // Top-right
    m_pockets[2] = Vector3(m_leftBound, 0.0f, m_bottomBound);  // Bottom-left
    m_pockets[3] = Vector3(m_rightBound, 0.0f, m_bottomBound); // Bottom-right
    
    // Side pockets
    m_pockets[4] = Vector3(m_leftBound, 0.0f, 0.0f);  // Left side
    m_pockets[5] = Vector3(m_rightBound, 0.0f, 0.0f); // Right side
}

bool Table::isBallInPocket(const Ball* ball) const {
    if (!ball) {
        return false;
    }
    
    Vector3 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    
    for (int i = 0; i < 6; ++i) {
        float distance = ballPos.distance(m_pockets[i]);
        if (distance < (m_pocketRadius - ballRadius)) {
            return true;
        }
    }
    
    return false;
}

void Table::handlePocketCollision(Ball* ball) {
    if (!ball) {
        return;
    }
    
    LOGI("Ball pocketed!");
    ball->setPocketed(true);
}

void Table::handleWallCollision(Ball* ball) {
    if (!ball) {
        return;
    }
    
    Vector3 position = ball->getPosition();
    Vector3 velocity = ball->getVelocity();
    float radius = ball->getRadius();
    
    bool collision = false;
    
    // Left wall
    if (position.x - radius < m_leftBound) {
        position.x = m_leftBound + radius;
        velocity.x = -velocity.x * 0.8f; // Bounce with some energy loss
        collision = true;
    }
    
    // Right wall
    if (position.x + radius > m_rightBound) {
        position.x = m_rightBound - radius;
        velocity.x = -velocity.x * 0.8f;
        collision = true;
    }
    
    // Top wall
    if (position.z - radius < m_topBound) {
        position.z = m_topBound + radius;
        velocity.z = -velocity.z * 0.8f;
        collision = true;
    }
    
    // Bottom wall
    if (position.z + radius > m_bottomBound) {
        position.z = m_bottomBound - radius;
        velocity.z = -velocity.z * 0.8f;
        collision = true;
    }
    
    if (collision) {
        ball->setPosition(position);
        ball->setVelocity(velocity);
    }
}