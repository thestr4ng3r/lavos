
set(DEMO_MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

macro(add_demo DEMO_NAME SOURCE_FILES_VAR)
	if(ANDROID)
		add_library(${DEMO_NAME} ${SOURCE_FILES})

		target_include_directories(${DEMO_NAME} PRIVATE
				${ANDROID_NDK}/sources/third_party/vulkan/src/include
				${ANDROID_NDK}/sources/android/native_app_glue
				${ANDROID_NDK}/sources/third_party/shaderc/include)

		target_link_libraries(${DEMO_NAME} engine)
	else()
		add_executable(${DEMO_NAME} ${SOURCE_FILES})

		target_link_libraries(${DEMO_NAME}
				demo_common
				engine
				glfw
				${Vulkan_LIBRARIES})
	endif()
endmacro()

add_subdirectory(common)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(gltf)
add_subdirectory(firstperson)