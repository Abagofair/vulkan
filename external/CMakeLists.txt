#CGLM HEADER ONLY

add_library(cglm INTERFACE)
add_library(stb INTERFACE)

target_include_directories(cglm INTERFACE cglm)
target_include_directories(stb INTERFACE stb)