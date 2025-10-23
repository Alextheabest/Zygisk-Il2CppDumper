#include <android/native_activity.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "renderer.h"
#include "game.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Pool", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Pool", __VA_ARGS__)

struct TouchState {
    bool isTouching = false;
    float startX = 0.0f;
    float startY = 0.0f;
    float currentX = 0.0f;
    float currentY = 0.0f;
};

static void handle_cmd(android_app* app, int32_t cmd) {
    auto renderer = reinterpret_cast<Renderer*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr) {
                renderer->init(app->window);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            renderer->shutdown();
            break;
        case APP_CMD_WINDOW_RESIZED:
        case APP_CMD_CONFIG_CHANGED:
            renderer->resize();
            break;
    }
}

static int32_t handle_input(android_app* app, AInputEvent* event) {
    TouchState* touch = reinterpret_cast<TouchState*>(app->savedState);
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:
                touch->isTouching = true;
                touch->startX = x;
                touch->startY = y;
                touch->currentX = x;
                touch->currentY = y;
                return 1;
            case AMOTION_EVENT_ACTION_MOVE:
                touch->currentX = x;
                touch->currentY = y;
                return 1;
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_CANCEL:
                touch->isTouching = false;
                touch->currentX = x;
                touch->currentY = y;
                return 1;
        }
    }
    return 0;
}

void android_main(android_app* app) {
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;

    Renderer renderer;
    Game game;
    TouchState touch;
    app->userData = &renderer;
    app->savedState = &touch;

    auto lastTime = std::chrono::steady_clock::now();

    while (true) {
        int events;
        android_poll_source* source;
        while (ALooper_pollAll(renderer.isInitialized() ? 0 : -1, nullptr, &events,
                               (void**)&source) >= 0) {
            if (source != nullptr) source->process(app, source);
            if (app->destroyRequested != 0) {
                renderer.shutdown();
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        // Handle cue input: drag from ball to aim
        if (!game.isBallsMoving() && touch.isTouching) {
            game.previewShot(touch.startX, touch.startY, touch.currentX, touch.currentY,
                             renderer.getRenderWidth(), renderer.getRenderHeight());
        }
        if (!game.isBallsMoving() && !touch.isTouching && (touch.startX != touch.currentX || touch.startY != touch.currentY)) {
            game.shootCueBall(touch.startX, touch.startY, touch.currentX, touch.currentY,
                              renderer.getRenderWidth(), renderer.getRenderHeight());
            // reset
            touch.startX = touch.currentX;
            touch.startY = touch.currentY;
        }

        game.update(dt);
        renderer.beginFrame();
        renderer.draw(game);
        renderer.endFrame();
    }
}
