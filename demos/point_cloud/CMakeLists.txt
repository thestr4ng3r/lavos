
find_package(PCL 1.3 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

set(SOURCE_FILES
		point_cloud.cpp)

add_executable(point_cloud ${SOURCE_FILES})
target_link_libraries(point_cloud
		${PCL_LIBRARIES}
		lavos
		lavos_shell_glfw
		glfw
		${Vulkan_LIBRARIES})