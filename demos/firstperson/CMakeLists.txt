
set(SOURCE_FILES
		firstperson.cpp)

add_executable(firstperson ${SOURCE_FILES})
target_link_libraries(firstperson
		lavos
		lavosframe
		glfw
		${Vulkan_LIBRARIES})