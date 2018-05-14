
set(SOURCE_FILES
		point_cloud.cpp)

add_executable(point_cloud ${SOURCE_FILES})
target_link_libraries(point_cloud
		lavos
		lavos_shell_glfw
		glfw
		${Vulkan_LIBRARIES})