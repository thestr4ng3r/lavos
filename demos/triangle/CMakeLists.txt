

set(SOURCE_FILES
		main.cpp
		triangle_application.cpp
		triangle_application.h)

set(GLSL_FILES
		glsl/triangle.vert
		glsl/triangle.frag)

set(GLSLC_EXECUTABLE glslc)

set(SPIRV_FILES "")
foreach(glsl_file ${GLSL_FILES})
	set(glsl_file "${CMAKE_CURRENT_SOURCE_DIR}/${glsl_file}")
	set(spirv_file "${glsl_file}.spv") # TODO: make this out of source
	list(APPEND SPIRV_FILES "${spirv_file}")
	add_custom_command(OUTPUT "${spirv_file}"
			COMMAND ${GLSLC_EXECUTABLE} "${glsl_file}" -o "${spirv_file}")
endforeach()

add_executable(triangle ${SOURCE_FILES} ${SPIRV_FILES})

target_link_libraries(triangle
		glfw
		${Vulkan_LIBRARIES})

