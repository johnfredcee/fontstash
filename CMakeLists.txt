cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(fontstash)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLFW REQUIRED)


file(GLOB C_FILES *.c)
file(GLOB H_FILES *.h)
add_executable(fontstash WIN32 ${C_FILES}  glad/src/glad.c ${H_FILES})
target_include_directories(fontstash PUBLIC ${GLFW_INCLUDE_DIR} glad/include)
target_link_libraries(fontstash ${GLFW_LIBRARY})