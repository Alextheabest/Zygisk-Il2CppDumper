#pragma once

#include <android/log.h>
#include <string>

#define LOG_TAG "EightBallPool"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

class Logger {
public:
    static void info(const std::string& message);
    static void error(const std::string& message);
    static void warning(const std::string& message);
    static void debug(const std::string& message);
    
    static void info(const char* message);
    static void error(const char* message);
    static void warning(const char* message);
    static void debug(const char* message);
};