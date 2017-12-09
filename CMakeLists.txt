cmake_minimum_required(VERSION 3.2)
project(lavos)

set(CMAKE_CXX_STANDARD 14)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lavos/include")

if(ANDROID)
    #include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/vulkan_wrapper")
    include_directories("${CMAKE_SOURCE_DIR}/android/vulkan-hpp"
                        "${CMAKE_SOURCE_DIR}/android/common")
    include_directories("${ANDROID_NDK}/sources/third_party/vulkan/src/include"
                        "${ANDROID_NDK}/sources/android/native_app_glue")

    set(Vulkan_LIBRARIES vulkan)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/android/common")

    add_subdirectory(android)
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")

    add_subdirectory(frame)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/frame/include")
endif()

add_subdirectory(lavos)

add_subdirectory(demos)