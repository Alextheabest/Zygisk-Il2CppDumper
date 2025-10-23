#pragma once

#include <android/native_window.h>
#include <android/asset_manager.h>
#include <vulkan/vulkan.h>
#include <memory>

class VulkanRenderer;
class PoolGame;

class Game {
public:
    Game();
    ~Game();

    bool initialize(ANativeWindow* window, AAssetManager* assetManager);
    void shutdown();
    
    void update();
    void render();
    void pause();
    void resume();
    void resize(int width, int height);
    void onTouch(float x, float y, int action);

private:
    bool m_initialized;
    bool m_paused;
    int m_width;
    int m_height;
    
    ANativeWindow* m_window;
    AAssetManager* m_assetManager;
    
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<PoolGame> m_poolGame;
    
    void updateInput();
    void updateGameLogic();
};