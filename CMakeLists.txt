cmake_minimum_required(VERSION 3.16)
project(PixiCozy CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure paths for locally downloaded 32-bit SDL2, SDL2_ttf, and stb
set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/i686-w64-mingw32")
set(SDL2_TTF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2_ttf/i686-w64-mingw32")
set(STB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb")

# Provide header search directories
include_directories(
    ${SDL2_PATH}/include
    ${SDL2_PATH}/include/SDL2
    ${SDL2_TTF_PATH}/include
    ${SDL2_TTF_PATH}/include/SDL2
    ${STB_PATH}
    src
)

# Provide library search directories for the linker
link_directories(
    ${SDL2_PATH}/lib
    ${SDL2_TTF_PATH}/lib
)

# Core Executable Target
add_executable(PixiCozy
    src/main.cpp
    src/canvas.cpp
    src/canvas.h
    src/layer.h
    src/history.cpp
    src/history.h
    src/ui.cpp
    src/ui.h
)

# Link SDL2 libraries in the correct order for MinGW
target_link_libraries(PixiCozy PRIVATE
    mingw32
    SDL2main
    SDL2
    SDL2_ttf
    comdlg32
)

# Copy runtime DLL files to the build folder automatically after compilation
add_custom_command(TARGET PixiCozy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
    $<TARGET_FILE_DIR:PixiCozy>
)

add_custom_command(TARGET PixiCozy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf.dll"
    $<TARGET_FILE_DIR:PixiCozy>
)
