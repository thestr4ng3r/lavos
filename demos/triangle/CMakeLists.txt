

set(SOURCE_FILES
		main.cpp
		triangle_application.cpp
		triangle_application.h)

add_executable(triangle ${SOURCE_FILES})

target_link_libraries(triangle
		glfw
		${Vulkan_LIBRARIES})