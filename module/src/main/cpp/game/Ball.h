#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

class VulkanRenderer;

class Ball {
public:
    enum class Type {
        CUE = 0,
        SOLID_1 = 1,
        SOLID_2 = 2,
        SOLID_3 = 3,
        SOLID_4 = 4,
        SOLID_5 = 5,
        SOLID_6 = 6,
        SOLID_7 = 7,
        STRIPED_8 = 8,
        STRIPED_9 = 9,
        STRIPED_10 = 10,
        STRIPED_11 = 11,
        STRIPED_12 = 12,
        STRIPED_13 = 13,
        STRIPED_14 = 14,
        STRIPED_15 = 15
    };
    
    Ball();
    ~Ball();
    
    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    void update();
    void render(VulkanRenderer* renderer);
    void updatePhysics(float deltaTime);
    
    // Getters and setters
    void setPosition(const Vector3& position);
    void setPosition(float x, float y, float z);
    Vector3 getPosition() const;
    
    void setVelocity(const Vector3& velocity);
    void setVelocity(float x, float y, float z);
    Vector3 getVelocity() const;
    
    void setType(Type type);
    Type getType() const;
    
    void setRadius(float radius);
    float getRadius() const;
    
    void setMass(float mass);
    float getMass() const;
    
    bool isMoving() const;
    bool isPocketed() const;
    void setPocketed(bool pocketed);
    
    // Physics
    void applyForce(const Vector3& force);
    void applyImpulse(const Vector3& impulse);
    void setFriction(float friction);
    float getFriction() const;
    
    // Collision
    bool checkCollision(const Ball& other) const;
    void resolveCollision(Ball& other);
    
    // Rendering
    Matrix4 getModelMatrix() const;

private:
    Type m_type;
    Vector3 m_position;
    Vector3 m_velocity;
    Vector3 m_angularVelocity;
    float m_radius;
    float m_mass;
    float m_friction;
    bool m_pocketed;
    bool m_initialized;
    
    // Rendering data
    Matrix4 m_modelMatrix;
    bool m_modelMatrixDirty;
    
    void updateModelMatrix();
    Vector3 getColor() const;
};