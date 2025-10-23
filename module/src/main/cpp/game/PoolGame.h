#pragma once

#include <vector>
#include <memory>

class VulkanRenderer;
class Table;
class Ball;
class Cue;
class Camera;

class PoolGame {
public:
    PoolGame();
    ~PoolGame();

    bool initialize(VulkanRenderer* renderer);
    void shutdown();
    
    void update();
    void render(VulkanRenderer* renderer);
    void resize(int width, int height);
    void onTouch(float x, float y, int action);

private:
    bool m_initialized;
    int m_width;
    int m_height;
    
    std::unique_ptr<Table> m_table;
    std::vector<std::unique_ptr<Ball>> m_balls;
    std::unique_ptr<Cue> m_cue;
    std::unique_ptr<Camera> m_camera;
    
    // Game state
    enum class GameState {
        PLACING_CUE,
        AIMING,
        SHOOTING,
        BALLS_MOVING,
        GAME_OVER
    };
    
    GameState m_gameState;
    int m_currentPlayer;
    bool m_gameStarted;
    
    void initializeBalls();
    void updatePhysics(float deltaTime);
    void checkCollisions();
    void updateGameState();
    void resetGame();
};