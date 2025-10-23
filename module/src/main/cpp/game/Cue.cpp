#include "Cue.h"
#include "Ball.h"
#include "vulkan/VulkanRenderer.h"
#include "utils/Logger.h"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "Cue"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Cue::Cue()
    : m_initialized(false)
    , m_targetBall(nullptr)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_direction(0.0f, 0.0f, 1.0f)
    , m_power(0.0f)
    , m_maxPower(10.0f)
    , m_aiming(false)
    , m_shooting(false)
    , m_lastTouchX(0.0f)
    , m_lastTouchY(0.0f)
    , m_touchDown(false)
{
}

Cue::~Cue() {
    shutdown();
}

bool Cue::initialize(VulkanRenderer* renderer) {
    if (!renderer) {
        LOGE("Renderer is null");
        return false;
    }
    
    m_initialized = true;
    LOGI("Cue initialized");
    return true;
}

void Cue::shutdown() {
    if (m_initialized) {
        m_initialized = false;
        LOGI("Cue shutdown");
    }
}

void Cue::update() {
    if (!m_initialized) {
        return;
    }
    
    if (m_aiming) {
        updateAiming();
    }
    
    if (m_shooting) {
        updateShooting();
    }
}

void Cue::render(VulkanRenderer* renderer) {
    if (!m_initialized || !renderer || !m_targetBall) {
        return;
    }
    
    if (!m_aiming && !m_shooting) {
        return;
    }
    
    // Set cue color (brown wood)
    renderer->setColor(0.6f, 0.3f, 0.1f, 1.0f);
    
    // Calculate cue position
    Vector3 ballPos = m_targetBall->getPosition();
    Vector3 cueOffset = m_direction * (2.0f + m_power * 0.5f);
    Vector3 cuePos = ballPos + cueOffset;
    
    // Set model matrix for cue
    Matrix4 cueMatrix = Matrix4::translation(cuePos);
    
    // Rotate cue to point towards ball
    Vector3 up = Vector3::up();
    Vector3 right = m_direction.cross(up).normalized();
    up = right.cross(m_direction).normalized();
    
    // Create rotation matrix
    Matrix4 rotation;
    rotation.m[0] = right.x;
    rotation.m[1] = right.y;
    rotation.m[2] = right.z;
    rotation.m[4] = up.x;
    rotation.m[5] = up.y;
    rotation.m[6] = up.z;
    rotation.m[8] = -m_direction.x;
    rotation.m[9] = -m_direction.y;
    rotation.m[10] = -m_direction.z;
    
    cueMatrix = cueMatrix * rotation;
    cueMatrix = cueMatrix * Matrix4::scale(Vector3(0.05f, 0.05f, 1.5f));
    
    renderer->setModelMatrix(cueMatrix);
    renderer->drawCylinder();
}

void Cue::onTouch(float x, float y, int action) {
    if (!m_initialized || !m_targetBall) {
        return;
    }
    
    switch (action) {
        case 0: // ACTION_DOWN
            m_touchDown = true;
            m_lastTouchX = x;
            m_lastTouchY = y;
            m_aiming = true;
            m_power = 0.0f;
            break;
            
        case 1: // ACTION_UP
            if (m_touchDown && m_aiming) {
                shoot();
            }
            m_touchDown = false;
            m_aiming = false;
            break;
            
        case 2: // ACTION_MOVE
            if (m_touchDown && m_aiming) {
                // Calculate power based on touch movement
                float deltaX = x - m_lastTouchX;
                float deltaY = y - m_lastTouchY;
                float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
                
                m_power = std::min(distance * 0.1f, m_maxPower);
                
                // Calculate direction based on touch position
                // This is simplified - in a real implementation, you'd convert screen to world coordinates
                m_direction = Vector3(deltaX * 0.01f, 0.0f, deltaY * 0.01f).normalized();
            }
            break;
    }
}

void Cue::setTargetBall(Ball* ball) {
    m_targetBall = ball;
    if (ball) {
        m_position = ball->getPosition() + Vector3(0.0f, 0.0f, 2.0f);
    }
}

void Cue::setPosition(const Vector3& position) {
    m_position = position;
}

void Cue::setDirection(const Vector3& direction) {
    m_direction = direction.normalized();
}

void Cue::setPower(float power) {
    m_power = std::min(power, m_maxPower);
}

Vector3 Cue::getPosition() const {
    return m_position;
}

Vector3 Cue::getDirection() const {
    return m_direction;
}

float Cue::getPower() const {
    return m_power;
}

bool Cue::isAiming() const {
    return m_aiming;
}

void Cue::shoot() {
    if (!m_targetBall || m_power <= 0.0f) {
        return;
    }
    
    LOGI("Shooting with power: %.2f", m_power);
    
    // Apply impulse to target ball
    Vector3 impulse = m_direction * m_power;
    m_targetBall->applyImpulse(impulse);
    
    m_shooting = true;
    m_aiming = false;
}

void Cue::reset() {
    m_aiming = false;
    m_shooting = false;
    m_power = 0.0f;
    m_touchDown = false;
}

void Cue::updateAiming() {
    // Update aiming logic
    if (m_targetBall) {
        m_position = m_targetBall->getPosition() + m_direction * (2.0f + m_power * 0.5f);
    }
}

void Cue::updateShooting() {
    // Update shooting animation
    m_power -= 0.1f;
    if (m_power <= 0.0f) {
        m_shooting = false;
        m_power = 0.0f;
    }
}

Matrix4 Cue::getModelMatrix() const {
    Vector3 ballPos = m_targetBall ? m_targetBall->getPosition() : Vector3::zero();
    Vector3 cueOffset = m_direction * (2.0f + m_power * 0.5f);
    Vector3 cuePos = ballPos + cueOffset;
    
    return Matrix4::translation(cuePos);
}