
set(SOURCE_FILES
		room.cpp)

add_executable(room ${SOURCE_FILES})
target_link_libraries(room
		lavos
		lavos_shell_glfw
		glfw
		${Vulkan_LIBRARIES})