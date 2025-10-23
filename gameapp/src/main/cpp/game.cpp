#include "game.h"
#include <android/log.h>
#include <algorithm>
#include <cmath>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Pool", __VA_ARGS__)

static float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

Game::Game() {
    resetTable(1920, 1080);
}

void Game::resetTable(uint32_t w, uint32_t h) {
    balls.clear();
    const float R = 0.04f; // in NDC-ish units
    // cue ball at left
    balls.push_back({-0.7f, 0.0f, 0,0, R, 1,1,1,1, false});
    cueIndex = 0;
    // triangle rack near right
    int row = 1;
    float startX = 0.4f;
    float startY = 0.0f;
    int colorIdx = 0;
    for (int r=0;r<5;++r) {
        for (int i=0;i<=r;++i) {
            float x = startX + r*R*2*0.9f;
            float y = startY + (i - r*0.5f) * R*2.1f;
            float colors[7][3] = {
                {1,0,0},{1,1,0},{0,1,0},{0,1,1},{0,0,1},{1,0,1},{1,0.5f,0}
            };
            auto c = colors[colorIdx++ % 7];
            balls.push_back({x,y,0,0,R,c[0],c[1],c[2],1,false});
        }
    }
}

bool Game::isBallsMoving() const {
    for (auto &b : balls) if (std::abs(b.vx) > 0.01f || std::abs(b.vy) > 0.01f) return true;
    return false;
}

void Game::previewShot(float sx, float sy, float cx, float cy, uint32_t w, uint32_t h) {
    float x1 = screenToTableX(sx, w);
    float y1 = screenToTableY(sy, h);
    float x2 = screenToTableX(cx, w);
    float y2 = screenToTableY(cy, h);
    previewX1 = x1; previewY1 = y1; previewX2 = x2; previewY2 = y2;
    hasPreview = true;
}

void Game::shootCueBall(float sx, float sy, float cx, float cy, uint32_t w, uint32_t h) {
    float x1 = screenToTableX(sx, w);
    float y1 = screenToTableY(sy, h);
    float x2 = screenToTableX(cx, w);
    float y2 = screenToTableY(cy, h);
    float dx = x1 - x2;
    float dy = y1 - y2;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 0.01f) return;
    dx /= len; dy /= len;
    float power = clampf(len, 0.0f, 1.2f);
    balls[cueIndex].vx = dx * power * 2.8f;
    balls[cueIndex].vy = dy * power * 2.8f;
    hasPreview = false;
}

void Game::update(float dt) {
    // integrate
    for (auto &b : balls) {
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        // friction
        b.vx *= std::max(0.0f, 1.0f - friction * dt);
        b.vy *= std::max(0.0f, 1.0f - friction * dt);
        if (std::abs(b.vx) < 0.02f) b.vx = 0;
        if (std::abs(b.vy) < 0.02f) b.vy = 0;
        // cushion collisions (simple AABB for table bounds)
        float R = b.radius;
        if (b.x < -1.0f + R) { b.x = -1.0f + R; b.vx = -b.vx; }
        if (b.x >  1.0f - R) { b.x =  1.0f - R; b.vx = -b.vx; }
        if (b.y < -0.5f + R) { b.y = -0.5f + R; b.vy = -b.vy; }
        if (b.y >  0.5f - R) { b.y =  0.5f - R; b.vy = -b.vy; }
    }

    resolveCollisions();
}

void Game::resolveCollisions() {
    const float restitution = 0.95f;
    for (size_t i=0;i<balls.size();++i) {
        for (size_t j=i+1;j<balls.size();++j) {
            auto &a = balls[i];
            auto &b = balls[j];
            if (a.pocketed || b.pocketed) continue;
            float dx = b.x - a.x; float dy = b.y - a.y;
            float dist2 = dx*dx + dy*dy;
            float r = a.radius + b.radius;
            if (dist2 < r*r) {
                float dist = std::sqrt(std::max(dist2, 1e-6f));
                float nx = dx / dist; float ny = dy / dist;
                float overlap = r - dist;
                // separate
                a.x -= nx * overlap * 0.5f; a.y -= ny * overlap * 0.5f;
                b.x += nx * overlap * 0.5f; b.y += ny * overlap * 0.5f;
                // relative velocity along normal
                float rvx = b.vx - a.vx; float rvy = b.vy - a.vy;
                float velAlongNormal = rvx*nx + rvy*ny;
                if (velAlongNormal > 0) continue;
                float jimp = -(1+restitution) * velAlongNormal;
                jimp *= 0.5f; // equal mass
                float impX = jimp * nx; float impY = jimp * ny;
                a.vx -= impX; a.vy -= impY;
                b.vx += impX; b.vy += impY;
            }
        }
    }
}

float Game::screenToTableX(float x, uint32_t w) const { return (x / (float)w) * 2.0f - 1.0f; }
float Game::screenToTableY(float y, uint32_t h) const { return 1.0f - (y / (float)h) * 2.0f; }

void Game::recordDrawCommands(VkCommandBuffer cmd, VkPipelineLayout layout, VkExtent2D extent) const {
    // Dynamic viewport/scissor
    VkViewport vp{}; vp.x = 0; vp.y = 0; vp.width = (float)extent.width; vp.height = (float)extent.height; vp.minDepth = 0; vp.maxDepth = 1;
    vkCmdSetViewport(cmd, 0, 1, &vp);
    VkRect2D sc{}; sc.offset = {0,0}; sc.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &sc);

    // Draw balls as small quads approximated by 2 triangles via push constants positions
    // Simpler: draw lines and points would require geometry; we'll draw colored full-screen triangles clipped by fragment shader (not implemented fully here)
    // For simplicity here, we draw nothing complex: future improvement would upload vertex buffers

    // Draw preview line as two points approximated by full-screen triangles is non-trivial without UBOs.
    // Minimal stub: no-op. In a fuller version, we'd use vertex buffers.
}
