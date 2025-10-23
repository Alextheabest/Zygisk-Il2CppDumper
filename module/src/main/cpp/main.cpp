#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include "game/Game.h"
#include "utils/Logger.h"

#define LOG_TAG "EightBallPool"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static Game* g_game = nullptr;
static ANativeWindow* g_window = nullptr;
static AAssetManager* g_assetManager = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnCreate(JNIEnv *env, jobject thiz, jobject assetManager) {
    LOGI("Game onCreate");
    g_assetManager = AAssetManager_fromJava(env, assetManager);
    if (g_assetManager == nullptr) {
        LOGE("Failed to get asset manager");
        return;
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnResume(JNIEnv *env, jobject thiz) {
    LOGI("Game onResume");
    if (g_game) {
        g_game->resume();
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnPause(JNIEnv *env, jobject thiz) {
    LOGI("Game onPause");
    if (g_game) {
        g_game->pause();
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnDestroy(JNIEnv *env, jobject thiz) {
    LOGI("Game onDestroy");
    if (g_game) {
        delete g_game;
        g_game = nullptr;
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnSurfaceCreated(JNIEnv *env, jobject thiz, jobject surface) {
    LOGI("Surface created");
    g_window = ANativeWindow_fromSurface(env, surface);
    if (g_window == nullptr) {
        LOGE("Failed to get native window from surface");
        return;
    }
    
    if (g_game == nullptr) {
        g_game = new Game();
        if (!g_game->initialize(g_window, g_assetManager)) {
            LOGE("Failed to initialize game");
            delete g_game;
            g_game = nullptr;
            return;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnSurfaceChanged(JNIEnv *env, jobject thiz, jint width, jint height) {
    LOGI("Surface changed: %dx%d", width, height);
    if (g_game) {
        g_game->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnSurfaceDestroyed(JNIEnv *env, jobject thiz) {
    LOGI("Surface destroyed");
    if (g_window) {
        ANativeWindow_release(g_window);
        g_window = nullptr;
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnTouch(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jint action) {
    if (g_game) {
        g_game->onTouch(x, y, action);
    }
}

JNIEXPORT void JNICALL
Java_com_eightballpool_game_MainActivity_nativeOnDrawFrame(JNIEnv *env, jobject thiz) {
    if (g_game) {
        g_game->update();
        g_game->render();
    }
}

} // extern "C"