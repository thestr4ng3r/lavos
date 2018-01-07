
set(SOURCE_FILES
		application.cpp
		application.h)

add_executable(firstperson ${SOURCE_FILES})
target_link_libraries(firstperson
		lavos
		lavosframe
		glfw
		${Vulkan_LIBRARIES})