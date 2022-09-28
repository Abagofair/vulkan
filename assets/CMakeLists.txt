file(GLOB_RECURSE RawAssets)
add_custom_target(Assets ALL DEPENDS ${RawAssets})
add_custom_command(
        TARGET Assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR})