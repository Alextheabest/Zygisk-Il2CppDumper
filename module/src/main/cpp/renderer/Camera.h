#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

class Camera {
public:
    Camera();
    ~Camera();

    // Position and orientation
    void setPosition(const Vector3& position);
    void setPosition(float x, float y, float z);
    Vector3 getPosition() const;
    
    void lookAt(const Vector3& target);
    void lookAt(float x, float y, float z);
    void lookAt(const Vector3& target, const Vector3& up);
    
    // Projection
    void setPerspective(float fov, float aspect, float near, float far);
    void setOrthographic(float left, float right, float bottom, float top, float near, float far);
    
    // Viewport
    void setAspectRatio(float aspect);
    void setFieldOfView(float fov);
    void setNearPlane(float near);
    void setFarPlane(float far);
    
    // Movement
    void translate(const Vector3& translation);
    void translate(float x, float y, float z);
    void rotate(const Vector3& axis, float angle);
    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    
    // Matrices
    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;
    
    // Getters
    Vector3 getForward() const;
    Vector3 getRight() const;
    Vector3 getUp() const;
    float getAspectRatio() const;
    float getFieldOfView() const;
    float getNearPlane() const;
    float getFarPlane() const;

private:
    Vector3 m_position;
    Vector3 m_target;
    Vector3 m_up;
    
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;
    
    bool m_perspective;
    
    void updateViewMatrix();
    void updateProjectionMatrix();
    
    mutable Matrix4 m_viewMatrix;
    mutable Matrix4 m_projectionMatrix;
    mutable bool m_viewDirty;
    mutable bool m_projectionDirty;
};