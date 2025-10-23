#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

class VulkanRenderer;
class Ball;

class Cue {
public:
    Cue();
    ~Cue();
    
    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    void update();
    void render(VulkanRenderer* renderer);
    void onTouch(float x, float y, int action);
    
    // Cue control
    void setTargetBall(Ball* ball);
    void setPosition(const Vector3& position);
    void setDirection(const Vector3& direction);
    void setPower(float power);
    
    // Getters
    Vector3 getPosition() const;
    Vector3 getDirection() const;
    float getPower() const;
    bool isAiming() const;
    
    // Shooting
    void shoot();
    void reset();

private:
    bool m_initialized;
    Ball* m_targetBall;
    Vector3 m_position;
    Vector3 m_direction;
    float m_power;
    float m_maxPower;
    bool m_aiming;
    bool m_shooting;
    
    // Touch input
    float m_lastTouchX;
    float m_lastTouchY;
    bool m_touchDown;
    
    void updateAiming();
    void updateShooting();
    Matrix4 getModelMatrix() const;
};