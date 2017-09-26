

option(ENGINE_IMPLEMENT_TINYGLTF "Compile implementation of tinygltf inside the engine." ON)
option(ENGINE_IMPLEMENT_STB_IMAGE "Compile implementation of stb_image inside the engine." ON)
option(ENGINE_IMPLEMENT_VMA "Compile implementation of Vulkan Memory Allocator inside the engine." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(GLSLC_EXECUTABLE glslc) # TODO: Write a find module or something

macro(add_spirv_shaders SPIRV_FILES_VAR GLSL_FILES_VAR)
	set(${SPIRV_FILES_VAR} "")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader/material")
	foreach(glsl_file ${${GLSL_FILES_VAR}})
		set(spirv_file "${CMAKE_CURRENT_BINARY_DIR}/shader/${glsl_file}.spv")
		set(glsl_file "${CMAKE_CURRENT_SOURCE_DIR}/glsl/${glsl_file}")
		list(APPEND ${SPIRV_FILES_VAR} "${spirv_file}")
		add_custom_command(OUTPUT "${spirv_file}"
				COMMAND ${GLSLC_EXECUTABLE} "${glsl_file}" -o "${spirv_file}"
				MAIN_DEPENDENCY "${glsl_file}")
	endforeach()
endmacro()

set(SOURCE_FILES
		src/thirdparty_impl.cpp
		src/engine.cpp
		include/engine.h
		include/vertex.h
		src/mesh.cpp
		include/mesh.h
		src/material/material.cpp
		include/material/material.h
		src/material/unlit_material.cpp
		include/material/unlit_material.h
		src/material/phong_material.cpp
		include/material/phong_material.h
		src/material/material_instance.cpp
		include/material/material_instance.h
		include/buffer.h
		src/image.cpp
		include/image.h
		src/texture.cpp
		include/texture.h
		include/glm_config.h
		src/asset_container.cpp
		include/asset_container.h
		src/renderer.cpp
		include/renderer.h
		src/shader_load.cpp
		include/shader_load.h
		src/scene.cpp
		include/scene.h
		src/node.cpp
		include/node.h
		include/component/component.h
		src/component/mesh_component.cpp
		include/component/mesh_component.h
		include/component/transform_component.h
		src/component/transform_component.cpp
		include/component/camera_component.h
		src/glm_stream.h
		src/component/directional_light_component.cpp
		include/component/directional_light_component.h
		src/component/fp_controller_component.cpp
		include/component/fp_controller_component.h)

set(GLSL_FILES
		material/unlit.vert
		material/unlit.frag
		material/phong.vert
		material/phong.frag)




if(ENGINE_IMPLEMENT_TINYGLTF)
	add_definitions(-DENGINE_IMPLEMENT_TINYGLTF)
endif()

if(ENGINE_IMPLEMENT_STB_IMAGE)
	add_definitions(-DENGINE_IMPLEMENT_STB_IMAGE)
endif()

if(ENGINE_IMPLEMENT_VMA)
	add_definitions(-DENGINE_IMPLEMENT_VMA)
endif()


add_spirv_shaders(SPIRV_FILES GLSL_FILES)

set(RESOURCES_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/spirv_resources")
set(RESOURCES_HEADER "${RESOURCES_SRC_DIR}/spirv_resources.h")
set(RESOURCES_SRC "${RESOURCES_SRC_DIR}/spirv_resources.cpp")
set(RESOURCES_RELATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shader")



add_library(engine STATIC ${SOURCE_FILES} ${RESOURCES_HEADERS} ${RESOURCES_SRC})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(engine ${Vulkan_LIBRARIES})



# resources

include(ResourceGenerator)

file(MAKE_DIRECTORY ${RESOURCES_SRC_DIR})
include_directories(${RESOURCES_SRC_DIR})

set(GENERATE_RESOURCES_ARGS
		BINARY
		RELATIVE_PATH ${RESOURCES_RELATIVE_PATH}
		HEADER_FILE ${RESOURCES_HEADER}
		HEADER_FILE_RELATIVE_PATH ${RESOURCES_SRC_DIR}
		SOURCE_FILE ${RESOURCES_SRC}
		FUNCTION_PREFIX "spirv_resources"
		INCLUDE_GUARD "SPIRV_RESOURCES"
		FILES ${SPIRV_FILES})

generate_resources(GENERATE_HEADER ${GENERATE_RESOURCES_ARGS})

set(RESOURCE_GENERATOR_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ResourceGenerator.cmake")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_resources_src.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/generate_resources_src.cmake")

add_custom_command(OUTPUT ${RESOURCES_SRC}
		COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/generate_resources_src.cmake"
		DEPENDS ${SPIRV_FILES})

add_custom_target(resources DEPENDS ${RESOURCES_HEADERS} ${RESOURCES_SRC})
add_dependencies(engine resources)


