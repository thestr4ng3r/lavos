cmake_minimum_required(VERSION 3.2)
project(vulkan)

set(CMAKE_CXX_STANDARD 11)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm")

if(ANDROID)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/vulkan_wrapper")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/vulkan-hpp")

    set(Vulkan_LIBRARIES vulkan)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/common")


    add_subdirectory(demos)
    add_subdirectory(android)
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkan-hpp")

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")

    add_subdirectory(demos)
endif()