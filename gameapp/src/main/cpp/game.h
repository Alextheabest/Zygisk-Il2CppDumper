#pragma once
#include <vector>
#include <vulkan/vulkan.h>

struct Ball {
    float x, y;
    float vx, vy;
    float radius;
    float color[3];
    bool pocketed = false;
};

class Game {
public:
    Game();

    void update(float dt);

    void previewShot(float sx, float sy, float cx, float cy, int w, int h);
    void shootCueBall(float sx, float sy, float cx, float cy, int w, int h);

    bool isBallsMoving() const { return moving; }

    // Record draw commands into the given command buffer.
    void recordDrawCommands(VkCommandBuffer cmd) const;

private:
    void resetTable(int w, int h);
    void ensureGeometry() const;

private:
    mutable std::vector<float> vertices; // interleaved: pos(2), color(3)
    std::vector<Ball> balls;
    bool moving = false;
    float tableWidth = 2.0f;  // world units
    float tableHeight = 1.0f; // world units
    float pixelToWorldX = 0.01f;
    float pixelToWorldY = 0.01f;
};
