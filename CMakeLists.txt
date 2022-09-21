#include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.16)
project(OhNoNo C)
set(CMAKE_C_STANDARD 99)

find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(external)
add_subdirectory(shaders)

add_executable(OhNoNo Main.c Window.c Window.h InternalVulkan.c InternalVulkan.h Utilities.h File.c File.h Timer.c Timer.h)

target_link_libraries(OhNoNo PUBLIC Vulkan::Vulkan SDL2::SDL2 SDL2::SDL2main)

add_dependencies(OhNoNo Shaders)