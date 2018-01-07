
option(BUILD_QT_DEMO "Build Qt Demo" OFF)

set(DEMO_MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

macro(add_demo DEMO_NAME SOURCE_FILES_VAR)
	if(ANDROID)
		add_library(${DEMO_NAME} ${SOURCE_FILES})

		target_include_directories(${DEMO_NAME} PRIVATE
				${ANDROID_NDK}/sources/third_party/vulkan/src/include
				${ANDROID_NDK}/sources/android/native_app_glue
				${ANDROID_NDK}/sources/third_party/shaderc/include)

		target_link_libraries(${DEMO_NAME}
		lavos)
	else()
		add_executable(${DEMO_NAME} ${SOURCE_FILES})

		target_link_libraries(${DEMO_NAME}
				demo_common
				lavos
				glfw
				${Vulkan_LIBRARIES})
	endif()
endmacro()

add_subdirectory(gltf)
add_subdirectory(firstperson)

if(BUILD_QT_DEMO)
	add_subdirectory(qt)
endif()