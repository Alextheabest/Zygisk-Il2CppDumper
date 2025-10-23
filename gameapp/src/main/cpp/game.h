#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

struct Ball {
    float x, y;
    float vx, vy;
    float radius;
    float r, g, b, a;
    bool pocketed;
};

class Game {
public:
    Game();

    void update(float dt);
    bool isBallsMoving() const;

    void previewShot(float sx, float sy, float cx, float cy, uint32_t w, uint32_t h);
    void shootCueBall(float sx, float sy, float cx, float cy, uint32_t w, uint32_t h);

    void recordDrawCommands(VkCommandBuffer cmd, VkPipelineLayout layout, VkExtent2D extent) const;

private:
    void resetTable(uint32_t w, uint32_t h);
    void resolveCollisions();

    float screenToTableX(float x, uint32_t w) const;
    float screenToTableY(float y, uint32_t h) const;

private:
    float tableWidth = 2.0f;  // NDC space [-1,1]
    float tableHeight = 1.0f;
    float friction = 0.35f; // decay per second

    std::vector<Ball> balls;
    int cueIndex = 0;

    // preview line
    bool hasPreview = false;
    float previewX1=0, previewY1=0, previewX2=0, previewY2=0;
};
