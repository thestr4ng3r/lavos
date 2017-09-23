cmake_minimum_required(VERSION 3.2)
project(vulkan)

set(CMAKE_CXX_STANDARD 14)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

if(ANDROID)
    #include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/vulkan_wrapper")
    include_directories("${CMAKE_SOURCE_DIR}/android/vulkan-hpp"
                        "${CMAKE_SOURCE_DIR}/android/common")
    include_directories("${ANDROID_NDK}/sources/third_party/vulkan/src/include"
                        "${ANDROID_NDK}/sources/android/native_app_glue")

    set(Vulkan_LIBRARIES vulkan)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/common")

    #set(ENGINE_IMPLEMENT_VMA OFF CACHE BOOL "Compile implementation of Vulkan Memory Allocator inside the engine.")

    add_subdirectory(android)
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")
endif()

add_subdirectory(engine)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/include")

add_subdirectory(demos)