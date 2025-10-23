#include "FileUtils.h"
#include "Logger.h"
#include <android/asset_manager.h>
#include <cstring>

AAssetManager* FileUtils::s_assetManager = nullptr;

void FileUtils::setAssetManager(AAssetManager* assetManager) {
    s_assetManager = assetManager;
}

std::vector<char> FileUtils::readFile(const std::string& filename) {
    std::vector<char> buffer;
    
    if (!s_assetManager) {
        Logger::error("Asset manager not set");
        return buffer;
    }
    
    AAsset* asset = AAssetManager_open(s_assetManager, filename.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        Logger::error("Failed to open asset: " + filename);
        return buffer;
    }
    
    size_t length = AAsset_getLength(asset);
    buffer.resize(length);
    
    int bytesRead = AAsset_read(asset, buffer.data(), length);
    if (bytesRead < 0) {
        Logger::error("Failed to read asset: " + filename);
        buffer.clear();
    }
    
    AAsset_close(asset);
    return buffer;
}

std::string FileUtils::readTextFile(const std::string& filename) {
    std::vector<char> buffer = readFile(filename);
    if (buffer.empty()) {
        return "";
    }
    
    return std::string(buffer.begin(), buffer.end());
}

bool FileUtils::fileExists(const std::string& filename) {
    if (!s_assetManager) {
        return false;
    }
    
    AAsset* asset = AAssetManager_open(s_assetManager, filename.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        return false;
    }
    
    AAsset_close(asset);
    return true;
}