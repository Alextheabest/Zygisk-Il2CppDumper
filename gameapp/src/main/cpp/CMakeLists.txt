cmake_minimum_required(VERSION 3.18.1)

project(poolvulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(poolvulkan SHARED
    main.cpp
    renderer.cpp
    game.cpp
)

# NDK libs
find_library(log-lib log)
find_library(android-lib android)
find_library(vulkan-lib vulkan)

# AHardwareBuffer usage for swapchain images (optional)
find_library(nativewindow-lib nativewindow)
find_library(native_app_glue-lib native_app_glue)

# native_app_glue is shipped as sources; we include it
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
add_library(app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)
target_include_directories(app-glue PUBLIC ${APP_GLUE_DIR})

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Build defines
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)

# Link
target_link_libraries(poolvulkan
    app-glue
    ${android-lib}
    ${log-lib}
    ${vulkan-lib}
    ${nativewindow-lib}
)
