#include "Camera.h"
#include <cmath>

Camera::Camera()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_target(0.0f, 0.0f, -1.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f)
    , m_aspect(16.0f / 9.0f)
    , m_near(0.1f)
    , m_far(1000.0f)
    , m_perspective(true)
    , m_viewDirty(true)
    , m_projectionDirty(true)
{
}

Camera::~Camera() {
}

void Camera::setPosition(const Vector3& position) {
    m_position = position;
    m_viewDirty = true;
}

void Camera::setPosition(float x, float y, float z) {
    setPosition(Vector3(x, y, z));
}

Vector3 Camera::getPosition() const {
    return m_position;
}

void Camera::lookAt(const Vector3& target) {
    m_target = target;
    m_viewDirty = true;
}

void Camera::lookAt(float x, float y, float z) {
    lookAt(Vector3(x, y, z));
}

void Camera::lookAt(const Vector3& target, const Vector3& up) {
    m_target = target;
    m_up = up;
    m_viewDirty = true;
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    m_fov = fov;
    m_aspect = aspect;
    m_near = near;
    m_far = far;
    m_perspective = true;
    m_projectionDirty = true;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far) {
    m_near = near;
    m_far = far;
    m_perspective = false;
    m_projectionDirty = true;
}

void Camera::setAspectRatio(float aspect) {
    m_aspect = aspect;
    m_projectionDirty = true;
}

void Camera::setFieldOfView(float fov) {
    m_fov = fov;
    m_projectionDirty = true;
}

void Camera::setNearPlane(float near) {
    m_near = near;
    m_projectionDirty = true;
}

void Camera::setFarPlane(float far) {
    m_far = far;
    m_projectionDirty = true;
}

void Camera::translate(const Vector3& translation) {
    m_position += translation;
    m_target += translation;
    m_viewDirty = true;
}

void Camera::translate(float x, float y, float z) {
    translate(Vector3(x, y, z));
}

void Camera::rotate(const Vector3& axis, float angle) {
    Vector3 forward = (m_target - m_position).normalized();
    Vector3 right = forward.cross(m_up).normalized();
    Vector3 up = right.cross(forward).normalized();
    
    // Create rotation matrix
    Matrix4 rotation = Matrix4::rotation(axis, angle);
    
    // Rotate forward vector
    forward = rotation.transformDirection(forward);
    up = rotation.transformDirection(up);
    
    // Update target
    m_target = m_position + forward;
    m_up = up;
    
    m_viewDirty = true;
}

void Camera::rotateX(float angle) {
    rotate(Vector3::right(), angle);
}

void Camera::rotateY(float angle) {
    rotate(Vector3::up(), angle);
}

void Camera::rotateZ(float angle) {
    rotate(Vector3::forward(), angle);
}

Matrix4 Camera::getViewMatrix() const {
    if (m_viewDirty) {
        updateViewMatrix();
        m_viewDirty = false;
    }
    return m_viewMatrix;
}

Matrix4 Camera::getProjectionMatrix() const {
    if (m_projectionDirty) {
        updateProjectionMatrix();
        m_projectionDirty = false;
    }
    return m_projectionMatrix;
}

Matrix4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

Vector3 Camera::getForward() const {
    return (m_target - m_position).normalized();
}

Vector3 Camera::getRight() const {
    return getForward().cross(m_up).normalized();
}

Vector3 Camera::getUp() const {
    return m_up;
}

float Camera::getAspectRatio() const {
    return m_aspect;
}

float Camera::getFieldOfView() const {
    return m_fov;
}

float Camera::getNearPlane() const {
    return m_near;
}

float Camera::getFarPlane() const {
    return m_far;
}

void Camera::updateViewMatrix() const {
    m_viewMatrix = Matrix4::lookAt(m_position, m_target, m_up);
}

void Camera::updateProjectionMatrix() const {
    if (m_perspective) {
        m_projectionMatrix = Matrix4::perspective(m_fov * M_PI / 180.0f, m_aspect, m_near, m_far);
    } else {
        // For orthographic, we need to define the bounds
        float left = -m_aspect;
        float right = m_aspect;
        float bottom = -1.0f;
        float top = 1.0f;
        m_projectionMatrix = Matrix4::orthographic(left, right, bottom, top, m_near, m_far);
    }
}