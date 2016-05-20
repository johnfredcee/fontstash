cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0004 OLD)

project (fontstash)

set (SRCS stb_truetype.c)
set (BUILD_EXAMPLE CACHE OFF "Build the example")
set (WITH_OPENGL3 OFF BOOL "With OpenGL3")
set (WITH_GLLOADGEN OFF "With glLoadGen cal convention")

if (WITH_GLLOADGEN)
    list (APPEND SRCS fontstash.hpp fontstash.cpp gl.cpp gl.hpp)
else (WITH_GLLOADGEN)
    list (APPEND SRCS fontstash.h fontstash.c)
endif (WITH_GLLOADGEN)

if (WITH_OPENGL3)
    add_definitions(-DSTH_OPENGL3)
endif (WITH_OPENGL3)

add_library(fontstash SHARED ${SRCS})
target_link_libraries(fontstash GL)

if (BUILD_EXAMPLE)
    find_package(SDL2 REQUIRED)

    add_executable(main main.cpp)
    target_include_directories(main PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(main fontstash ${SDL2_LIBRARIES} SDL2_image)

    add_custom_target(run
        COMMAND main
        DEPENDS main
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    )
endif (BUILD_EXAMPLE)

