

set(SOURCE_FILES
		triangle_application.cpp
		triangle_application.h)

set(GLSL_FILES
		triangle.vert
		triangle.frag)

set(GLSLC_EXECUTABLE glslc)

set(SPIRV_FILES "")
foreach(glsl_file ${GLSL_FILES})
	set(spirv_file "${CMAKE_CURRENT_BINARY_DIR}/shader/${glsl_file}.spv")
	set(glsl_file "${CMAKE_CURRENT_SOURCE_DIR}/glsl/${glsl_file}")
	list(APPEND SPIRV_FILES "${spirv_file}")
	add_custom_command(OUTPUT "${spirv_file}"
			COMMAND ${GLSLC_EXECUTABLE} "${glsl_file}" -o "${spirv_file}"
			MAIN_DEPENDENCY ${glsl_file})
endforeach()

if(ANDROID)
	add_library(triangle ${SOURCE_FILES} ${SPIRV_FILES})

    target_include_directories(triangle PRIVATE
                              ${ANDROID_NDK}/sources/third_party/vulkan/src/include
                              ${CMAKE_SOURCE_DIR}/android/vulkan-hpp
                              ${ANDROID_NDK}/sources/android/native_app_glue
                              ${ANDROID_NDK}/sources/third_party/shaderc/include)
else()
	add_executable(triangle ${SOURCE_FILES} main.cpp ${SPIRV_FILES})

	target_link_libraries(triangle
			glfw
			${Vulkan_LIBRARIES})
endif()
