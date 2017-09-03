
set(GLSLC_EXECUTABLE glslc)

set(DEMO_MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

macro(add_spirv_shaders SPIRV_FILES_VAR GLSL_FILES_VAR)
	set(${SPIRV_FILES_VAR} "")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader")
	foreach(glsl_file ${${GLSL_FILES_VAR}})
		set(spirv_file "${CMAKE_CURRENT_BINARY_DIR}/shader/${glsl_file}.spv")
		set(glsl_file "${CMAKE_CURRENT_SOURCE_DIR}/glsl/${glsl_file}")
		list(APPEND SPIRV_FILES_VAR "${spirv_file}")
		add_custom_command(OUTPUT "${spirv_file}"
				COMMAND ${GLSLC_EXECUTABLE} "${glsl_file}" -o "${spirv_file}"
				MAIN_DEPENDENCY "${glsl_file}")
	endforeach()
endmacro()

macro(add_demo DEMO_NAME SOURCE_FILES_VAR GLSL_FILES_VAR)
	add_spirv_shaders(SPIRV_FILES ${GLSL_FILES_VAR})

	if(ANDROID)
		add_library(${DEMO_NAME} ${SOURCE_FILES} ${SPIRV_FILES})

		target_include_directories(${DEMO_NAME} PRIVATE
				${ANDROID_NDK}/sources/third_party/vulkan/src/include
				${CMAKE_SOURCE_DIR}/android/vulkan-hpp
				${ANDROID_NDK}/sources/android/native_app_glue
				${ANDROID_NDK}/sources/third_party/shaderc/include)
	else()
		add_executable(${DEMO_NAME} ${SOURCE_FILES} ${SPIRV_FILES})

		target_link_libraries(${DEMO_NAME}
				demo_common
				engine
				glfw
				${Vulkan_LIBRARIES})
	endif()
endmacro()

add_subdirectory(common)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(triangle)
add_subdirectory(vertexbuffer)