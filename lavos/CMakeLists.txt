

option(LAVOS_IMPLEMENT_TINYGLTF "Compile implementation of tinygltf inside lavos." ON)
option(LAVOS_IMPLEMENT_STB_IMAGE "Compile implementation of stb_image inside lavos." ON)
option(LAVOS_IMPLEMENT_VMA "Compile implementation of Vulkan Memory Allocator inside lavos." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Shaderc REQUIRED)

macro(add_spirv_shaders SPIRV_FILES_VAR GLSL_FILES_VAR)
	set(${SPIRV_FILES_VAR} "")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader/material")
	foreach(glsl_file ${${GLSL_FILES_VAR}})
		set(spirv_file "${CMAKE_CURRENT_BINARY_DIR}/shader/${glsl_file}.spv")
		set(glsl_file "${CMAKE_CURRENT_SOURCE_DIR}/glsl/${glsl_file}")
		list(APPEND ${SPIRV_FILES_VAR} "${spirv_file}")
		add_custom_command(OUTPUT "${spirv_file}"
				COMMAND ${Shaderc_GLSLC} "${glsl_file}" -o "${spirv_file}"
				MAIN_DEPENDENCY "${glsl_file}")
	endforeach()
endmacro()

set(SOURCE_FILES
		src/thirdparty_impl.cpp
		src/engine.cpp
		include/lavos/engine.h
		include/lavos/vertex.h
		src/mesh.cpp
		include/lavos/mesh.h
		src/material/material.cpp
		include/lavos/material/material.h
		src/material/unlit_material.cpp
		include/lavos/material/unlit_material.h
		src/material/phong_material.cpp
		include/lavos/material/phong_material.h
		src/material/gouraud_material.cpp
		include/lavos/material/gouraud_material.h
		src/material/material_instance.cpp
		include/lavos/material/material_instance.h
		include/lavos/buffer.h
		src/image.cpp
		include/lavos/image.h
		src/texture.cpp
		include/lavos/texture.h
		include/lavos/glm_config.h
		src/asset_container.cpp
		include/lavos/asset_container.h
		src/renderer.cpp
		include/lavos/renderer.h
		src/shader_load.cpp
		include/lavos/shader_load.h
		src/scene.cpp
		include/lavos/scene.h
		src/node.cpp
		include/lavos/node.h
		include/lavos/component/component.h
		src/component/mesh_component.cpp
		include/lavos/component/mesh_component.h
		include/lavos/component/transform_component.h
		src/component/transform_component.cpp
		include/lavos/component/camera_component.h
		src/glm_stream.h
		src/component/directional_light_component.cpp
		include/lavos/component/directional_light_component.h
		src/component/fp_controller_component.cpp
		include/lavos/component/fp_controller_component.h
		include/lavos/render_target.h
		src/render_target.cpp
		include/lavos/swapchain.h
		src/swapchain.cpp
		include/lavos/material/point_cloud_material.h
		src/material/point_cloud_material.cpp
		include/lavos/point_cloud.h
		include/lavos/component/point_cloud_component.h
		include/lavos/renderable.h
		include/lavos/component/spot_light_component.h
		src/component/spot_light_component.cpp
		include/lavos/spot_light_shadow.h
		src/spot_light_shadow.cpp
		src/buffer.cpp
		include/lavos/render_config.h
		src/render_config.cpp)

set(GLSL_FILES
		material/unlit.vert
		material/unlit.frag
		material/phong.vert
		material/phong.frag
		material/gouraud.vert
		material/gouraud.frag
		material/point_cloud.vert
		material/point_cloud.frag
		material/shadow.vert
		material/shadow.frag)




if(LAVOS_IMPLEMENT_TINYGLTF)
	add_definitions(-DLAVOS_IMPLEMENT_TINYGLTF)
endif()

if(LAVOS_IMPLEMENT_STB_IMAGE)
	add_definitions(-DLAVOS_IMPLEMENT_STB_IMAGE)
endif()

if(LAVOS_IMPLEMENT_VMA)
	add_definitions(-DLAVOS_IMPLEMENT_VMA)
endif()


add_spirv_shaders(SPIRV_FILES GLSL_FILES)

set(RESOURCES_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/spirv_resources")
set(RESOURCES_HEADER "${RESOURCES_SRC_DIR}/spirv_resources.h")
set(RESOURCES_SRC "${RESOURCES_SRC_DIR}/spirv_resources.cpp")
set(RESOURCES_RELATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shader")



add_library(lavos SHARED ${SOURCE_FILES} ${RESOURCES_HEADERS} ${RESOURCES_SRC})
target_link_libraries(lavos ${Vulkan_LIBRARIES})
target_include_directories(lavos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")



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
add_dependencies(lavos resources)


