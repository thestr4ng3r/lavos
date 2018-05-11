
set(SOURCE_FILES
		gltf.cpp)

add_executable(gltf ${SOURCE_FILES})
target_link_libraries(gltf
		lavos
		lavos_shell_glfw
		glfw
		${Vulkan_LIBRARIES})