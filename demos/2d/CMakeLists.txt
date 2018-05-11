
set(SOURCE_FILES
		2d.cpp)

add_executable(2d ${SOURCE_FILES})
target_link_libraries(2d
		lavos
		lavos_shell_glfw
		glfw
		${Vulkan_LIBRARIES})