#include "Logger.h"

void Logger::info(const std::string& message) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", message.c_str());
}

void Logger::error(const std::string& message) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", message.c_str());
}

void Logger::warning(const std::string& message) {
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", message.c_str());
}

void Logger::debug(const std::string& message) {
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", message.c_str());
}

void Logger::info(const char* message) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", message);
}

void Logger::error(const char* message) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", message);
}

void Logger::warning(const char* message) {
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", message);
}

void Logger::debug(const char* message) {
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", message);
}