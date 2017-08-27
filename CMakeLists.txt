cmake_minimum_required(VERSION 3.7)
project(vulkan)

set(CMAKE_CXX_STANDARD 11)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkan-hpp")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")

add_subdirectory(demos)