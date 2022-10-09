#CGLM HEADER ONLY

add_library(cglm INTERFACE)
add_library(stb INTERFACE)
add_library(argtable3 INTERFACE)
add_library(cgltf INTERFACE)

target_include_directories(cglm INTERFACE cglm)
target_include_directories(stb INTERFACE stb)
target_include_directories(argtable3 INTERFACE argtable3)
target_include_directories(cgltf INTERFACE cgltf)