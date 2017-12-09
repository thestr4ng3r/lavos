
set(SOURCE_FILES
		application.cpp
		application.h)

#add_demo(gltf SOURCE_FILES)
add_executable(gltf ${SOURCE_FILES})
target_link_libraries(gltf
		lavos
		lavosframe
		glfw
		${Vulkan_LIBRARIES})