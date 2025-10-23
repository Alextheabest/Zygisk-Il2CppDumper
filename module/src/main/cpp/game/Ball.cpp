#include "Ball.h"
#include "vulkan/VulkanRenderer.h"
#include "utils/Logger.h"
#include <android/log.h>
#include <cmath>

#define LOG_TAG "Ball"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Ball::Ball()
    : m_type(Type::CUE)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f, 0.0f)
    , m_angularVelocity(0.0f, 0.0f, 0.0f)
    , m_radius(0.5f)
    , m_mass(1.0f)
    , m_friction(0.98f)
    , m_pocketed(false)
    , m_initialized(false)
    , m_modelMatrixDirty(true)
{
}

Ball::~Ball() {
    shutdown();
}

bool Ball::initialize(VulkanRenderer* renderer) {
    if (!renderer) {
        LOGE("Renderer is null");
        return false;
    }
    
    m_initialized = true;
    LOGI("Ball initialized");
    return true;
}

void Ball::shutdown() {
    if (m_initialized) {
        m_initialized = false;
        LOGI("Ball shutdown");
    }
}

void Ball::update() {
    if (!m_initialized) {
        return;
    }
    
    updateModelMatrix();
}

void Ball::render(VulkanRenderer* renderer) {
    if (!m_initialized || !renderer || m_pocketed) {
        return;
    }
    
    // Set model matrix
    renderer->setModelMatrix(getModelMatrix());
    
    // Set color based on ball type
    Vector3 color = getColor();
    renderer->setColor(color.x, color.y, color.z, 1.0f);
    
    // Render sphere (simplified - in real implementation, you'd render a proper sphere mesh)
    renderer->drawSphere(m_radius);
}

void Ball::updatePhysics(float deltaTime) {
    if (m_pocketed) {
        return;
    }
    
    // Apply friction
    m_velocity *= m_friction;
    m_angularVelocity *= m_friction;
    
    // Update position
    m_position += m_velocity * deltaTime;
    
    // Stop if velocity is very small
    if (m_velocity.lengthSquared() < 0.001f) {
        m_velocity = Vector3::zero();
    }
    
    m_modelMatrixDirty = true;
}

void Ball::setPosition(const Vector3& position) {
    m_position = position;
    m_modelMatrixDirty = true;
}

void Ball::setPosition(float x, float y, float z) {
    setPosition(Vector3(x, y, z));
}

Vector3 Ball::getPosition() const {
    return m_position;
}

void Ball::setVelocity(const Vector3& velocity) {
    m_velocity = velocity;
}

void Ball::setVelocity(float x, float y, float z) {
    setVelocity(Vector3(x, y, z));
}

Vector3 Ball::getVelocity() const {
    return m_velocity;
}

void Ball::setType(Type type) {
    m_type = type;
}

Ball::Type Ball::getType() const {
    return m_type;
}

void Ball::setRadius(float radius) {
    m_radius = radius;
    m_modelMatrixDirty = true;
}

float Ball::getRadius() const {
    return m_radius;
}

void Ball::setMass(float mass) {
    m_mass = mass;
}

float Ball::getMass() const {
    return m_mass;
}

bool Ball::isMoving() const {
    return m_velocity.lengthSquared() > 0.001f;
}

bool Ball::isPocketed() const {
    return m_pocketed;
}

void Ball::setPocketed(bool pocketed) {
    m_pocketed = pocketed;
    if (pocketed) {
        m_velocity = Vector3::zero();
        m_angularVelocity = Vector3::zero();
    }
}

void Ball::applyForce(const Vector3& force) {
    m_velocity += force / m_mass;
}

void Ball::applyImpulse(const Vector3& impulse) {
    m_velocity += impulse / m_mass;
}

void Ball::setFriction(float friction) {
    m_friction = friction;
}

float Ball::getFriction() const {
    return m_friction;
}

bool Ball::checkCollision(const Ball& other) const {
    if (m_pocketed || other.m_pocketed) {
        return false;
    }
    
    float distance = m_position.distance(other.m_position);
    return distance < (m_radius + other.m_radius);
}

void Ball::resolveCollision(Ball& other) {
    if (m_pocketed || other.m_pocketed) {
        return;
    }
    
    Vector3 collisionVector = m_position - other.m_position;
    float distance = collisionVector.length();
    
    if (distance == 0.0f) {
        return;
    }
    
    Vector3 collisionNormal = collisionVector / distance;
    Vector3 relativeVelocity = m_velocity - other.m_velocity;
    float velocityAlongNormal = relativeVelocity.dot(collisionNormal);
    
    // Do not resolve if velocities are separating
    if (velocityAlongNormal > 0) {
        return;
    }
    
    // Calculate restitution
    float restitution = 0.8f; // Bounciness
    
    // Calculate impulse scalar
    float impulseScalar = -(1 + restitution) * velocityAlongNormal;
    impulseScalar /= (1.0f / m_mass + 1.0f / other.m_mass);
    
    // Apply impulse
    Vector3 impulse = impulseScalar * collisionNormal;
    m_velocity += impulse / m_mass;
    other.m_velocity -= impulse / other.m_mass;
}

Matrix4 Ball::getModelMatrix() const {
    if (m_modelMatrixDirty) {
        updateModelMatrix();
    }
    return m_modelMatrix;
}

void Ball::updateModelMatrix() const {
    m_modelMatrix = Matrix4::translation(m_position) * Matrix4::scale(m_radius);
    m_modelMatrixDirty = false;
}

Vector3 Ball::getColor() const {
    switch (m_type) {
        case Type::CUE:
            return Vector3(1.0f, 1.0f, 1.0f); // White
        case Type::SOLID_1:
            return Vector3(1.0f, 0.0f, 0.0f); // Red
        case Type::SOLID_2:
            return Vector3(0.0f, 0.0f, 1.0f); // Blue
        case Type::SOLID_3:
            return Vector3(1.0f, 1.0f, 0.0f); // Yellow
        case Type::SOLID_4:
            return Vector3(0.0f, 1.0f, 0.0f); // Green
        case Type::SOLID_5:
            return Vector3(1.0f, 0.0f, 1.0f); // Magenta
        case Type::SOLID_6:
            return Vector3(1.0f, 0.5f, 0.0f); // Orange
        case Type::SOLID_7:
            return Vector3(0.5f, 0.0f, 0.5f); // Purple
        case Type::STRIPED_8:
            return Vector3(0.0f, 0.0f, 0.0f); // Black
        case Type::STRIPED_9:
            return Vector3(1.0f, 0.0f, 0.0f); // Red striped
        case Type::STRIPED_10:
            return Vector3(0.0f, 0.0f, 1.0f); // Blue striped
        case Type::STRIPED_11:
            return Vector3(1.0f, 1.0f, 0.0f); // Yellow striped
        case Type::STRIPED_12:
            return Vector3(0.0f, 1.0f, 0.0f); // Green striped
        case Type::STRIPED_13:
            return Vector3(1.0f, 0.0f, 1.0f); // Magenta striped
        case Type::STRIPED_14:
            return Vector3(1.0f, 0.5f, 0.0f); // Orange striped
        case Type::STRIPED_15:
            return Vector3(0.5f, 0.0f, 0.5f); // Purple striped
        default:
            return Vector3(0.5f, 0.5f, 0.5f); // Gray
    }
}