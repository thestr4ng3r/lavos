cmake_minimum_required(VERSION 3.2)
project(lavos)

set(CMAKE_CXX_STANDARD 14)

option(LAVOS_BUILD_SHELL_GLFW "Build GLFW Lavos Shell library" ON)
option(LAVOS_BUILD_SHELL_QT "Build Qt Lavos Shell library" OFF)

option(LAVOS_BUILD_DEMOS "Build Demos" ON)
option(LAVOS_BUILD_QT_DEMOS "Build Qt Demo" OFF)
option(LAVOS_BUILD_POINT_CLOUD_DEMO "Build Point Cloud Demo (requires PCL)" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vma/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mikktspace")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

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
endif()

find_package(VulkanHPP REQUIRED)
include_directories(${VulkanHPP_INCLUDE_DIRS})

add_subdirectory(lavos)
add_subdirectory(shell)

if(LAVOS_BUILD_DEMOS)
	add_subdirectory(demos)
endif()
