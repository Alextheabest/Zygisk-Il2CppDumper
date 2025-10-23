cmake_minimum_required(VERSION 3.18.1)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(android REQUIRED)
find_library(log-lib log)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(GLESv3-lib GLESv3)

# Vulkan
find_library(vulkan-lib vulkan)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
    ${CMAKE_CURRENT_SOURCE_DIR}/game
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Source files
set(SOURCES
    main.cpp
    game/Game.cpp
    game/Game.h
    game/PoolGame.cpp
    game/PoolGame.h
    game/Ball.cpp
    game/Ball.h
    game/Table.cpp
    game/Table.h
    game/Cue.cpp
    game/Cue.h
    game/Physics.cpp
    game/Physics.h
    vulkan/VulkanRenderer.cpp
    vulkan/VulkanRenderer.h
    vulkan/VulkanDevice.cpp
    vulkan/VulkanDevice.h
    vulkan/VulkanSwapchain.cpp
    vulkan/VulkanSwapchain.h
    vulkan/VulkanBuffer.cpp
    vulkan/VulkanBuffer.h
    vulkan/VulkanTexture.cpp
    vulkan/VulkanTexture.h
    vulkan/VulkanShader.cpp
    vulkan/VulkanShader.h
    vulkan/VulkanPipeline.cpp
    vulkan/VulkanPipeline.h
    vulkan/VulkanCommandBuffer.cpp
    vulkan/VulkanCommandBuffer.h
    vulkan/VulkanMemory.cpp
    vulkan/VulkanMemory.h
    math/Vector3.cpp
    math/Vector3.h
    math/Matrix4.cpp
    math/Matrix4.h
    math/Quaternion.cpp
    math/Quaternion.h
    renderer/Camera.cpp
    renderer/Camera.h
    renderer/Mesh.cpp
    renderer/Mesh.h
    renderer/Material.cpp
    renderer/Material.h
    renderer/Model.cpp
    renderer/Model.h
    utils/Logger.cpp
    utils/Logger.h
    utils/FileUtils.cpp
    utils/FileUtils.h
)

# Create the library
add_library(${MODULE_NAME} SHARED ${SOURCES})

# Link libraries
target_link_libraries(${MODULE_NAME}
    ${log-lib}
    ${android-lib}
    ${EGL-lib}
    ${GLESv3-lib}
    ${vulkan-lib}
)

# Android specific settings
set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

# Strip symbols in release build
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${MODULE_NAME}.so"
    )
endif ()
