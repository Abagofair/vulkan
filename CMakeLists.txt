#include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.16)
project(OhNoNo C)
set(CMAKE_C_STANDARD 99)

find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Argtable3 CONFIG REQUIRED)
find_package(cJSON CONFIG REQUIRED)

add_subdirectory(external)
add_subdirectory(shaders)
add_subdirectory(textures)
add_subdirectory(assets)

add_executable(AssetCreator AssetCreator.c File.c File.h AssetStructures.h)
target_link_libraries(AssetCreator argtable3::argtable3 cgltf cjson)
target_include_directories(AssetCreator PRIVATE external/argtable3)

add_executable(OhNoNo Main.c Window.c Window.h InternalVulkan.c InternalVulkan.h Utilities.h File.c File.h Timer.c Timer.h AssetStructures.h AssetManager.c AssetManager.h)
target_link_libraries(OhNoNo PUBLIC Vulkan::Vulkan SDL2::SDL2 SDL2::SDL2main)
add_dependencies(OhNoNo Shaders Images)