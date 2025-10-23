#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

class VulkanRenderer;
class Ball;

class Table {
public:
    Table();
    ~Table();
    
    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    void update();
    void render(VulkanRenderer* renderer);
    void resize(int width, int height);
    
    // Collision detection
    void checkBallCollision(Ball* ball);
    
    // Getters
    float getWidth() const;
    float getHeight() const;
    float getPocketRadius() const;
    
    // Table dimensions
    void setWidth(float width);
    void setHeight(float height);
    void setPocketRadius(float radius);

private:
    bool m_initialized;
    float m_width;
    float m_height;
    float m_pocketRadius;
    
    // Pocket positions
    Vector3 m_pockets[6];
    
    // Table boundaries
    float m_leftBound;
    float m_rightBound;
    float m_topBound;
    float m_bottomBound;
    
    void initializePockets();
    bool isBallInPocket(const Ball* ball) const;
    void handlePocketCollision(Ball* ball);
    void handleWallCollision(Ball* ball);
};