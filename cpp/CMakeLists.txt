cmake_minimum_required(VERSION 3.10)
project(cpp_game)

set(CMAKE_CXX_STANDARD 20)

# SDL2
set(SDL2_DIR "C:/libs/SDL2-2.24.2")
set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include")
set(SDL2_LIB_DIR "${SDL2_DIR}/lib/x64")
# C:/libs/SDL2-2.24.2/include
# C:/libs/SDL2-2.24.2/lib/x64
# SDL2_ttf
set(SDL2_TTF_DIR "C:/libs/SDL2_ttf-2.22.0")
set(SDL2_TTF_INCLUDE_DIR "${SDL2_TTF_DIR}/include")
set(SDL2_TTF_LIB_DIR "${SDL2_TTF_DIR}/lib/x64")

# Include both SDL2 and SDL2_ttf headers
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR} ${SDL2_TTF_LIB_DIR})

file(GLOB SOURCES
    *.cpp
    *.h
)

add_executable(cpp_game ${SOURCES})

# Link against both SDL2 and SDL2_ttf libraries
target_link_libraries(cpp_game SDL2.lib SDL2main.lib SDL2_ttf.lib)

# Copy both SDL2 and SDL2_ttf DLLs after build
add_custom_command(TARGET cpp_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        "${SDL2_TTF_LIB_DIR}/SDL2_ttf.dll"
        $<TARGET_FILE_DIR:cpp_game>
)

# Create resources directory and copy font file
add_custom_command(TARGET cpp_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:cpp_game>/resources

)

# Copy font file to resources directory
add_custom_command(TARGET cpp_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/MinecraftTen-VGORe.ttf"
        $<TARGET_FILE_DIR:cpp_game>/resources/
)
