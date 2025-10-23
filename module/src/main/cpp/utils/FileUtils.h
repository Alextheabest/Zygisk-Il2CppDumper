#pragma once

#include <android/asset_manager.h>
#include <string>
#include <vector>

class FileUtils {
public:
    static void setAssetManager(AAssetManager* assetManager);
    static std::vector<char> readFile(const std::string& filename);
    static std::string readTextFile(const std::string& filename);
    static bool fileExists(const std::string& filename);

private:
    static AAssetManager* s_assetManager;
};