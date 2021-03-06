
option(LAVOS_IMPLEMENT_TINYGLTF "Compile implementation of tinygltf inside lavos." ON)
option(LAVOS_IMPLEMENT_STB_IMAGE "Compile implementation of stb_image inside lavos." ON)
option(LAVOS_IMPLEMENT_VMA "Compile implementation of Vulkan Memory Allocator inside lavos." ON)

option(LAVOS_ENABLE_RENDERDOC "Enable RenderDoc integration." OFF)
option(LAVOS_ENABLE_VK_NAMES "Enable naming vulkan objects for debugging." OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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
		include/lavos/component/camera.h
		src/glm_stream.h
		src/component/directional_light.cpp
		include/lavos/component/directional_light.h
		src/component/fp_controller.cpp
		include/lavos/component/fp_controller.h
		include/lavos/render_target.h
		src/render_target.cpp
		include/lavos/swapchain.h
		src/swapchain.cpp
		include/lavos/material/point_cloud_material.h
		src/material/point_cloud_material.cpp
		include/lavos/point_cloud.h
		include/lavos/component/point_cloud_component.h
		include/lavos/renderable.h
		include/lavos/component/spot_light.h
		src/component/spot_light.cpp
		include/lavos/spot_light_shadow.h
		src/spot_light_shadow.cpp
		src/buffer.cpp
		include/lavos/render_config.h
		src/render_config.cpp
		include/lavos/material_pipeline_manager.h
		src/material_pipeline_manager.cpp
		include/lavos/sub_renderer.h
		src/sub_renderer.cpp
		include/lavos/spot_light_shadow_renderer.h
		src/spot_light_shadow_renderer.cpp
		include/lavos/log.h
		src/log.cpp
		include/lavos/vk_util.h
		include/lavos/light_collection.h
		src/light_collection.cpp)

set(GLSL_FILES
		material/unlit.vf.shader
		material/phong.vf.shader
		material/gouraud.vf.shader
		material/point_cloud.vf.shader
		material/shadow.vf.shader)




if(LAVOS_IMPLEMENT_TINYGLTF)
	add_definitions(-DLAVOS_IMPLEMENT_TINYGLTF)
endif()

if(LAVOS_IMPLEMENT_STB_IMAGE)
	add_definitions(-DLAVOS_IMPLEMENT_STB_IMAGE)
endif()

if(LAVOS_IMPLEMENT_VMA)
	add_definitions(-DLAVOS_IMPLEMENT_VMA)
endif()

if(LAVOS_ENABLE_RENDERDOC)
	add_definitions(-DLAVOS_ENABLE_RENDERDOC)
endif()

if(LAVOS_ENABLE_VK_NAMES)
	add_definitions(-DLAVOS_ENABLE_VK_NAMES)
endif()

include(ShaderResources)

add_spirv_shaders(VAR SPIRV_FILES
		BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glsl"
		TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/shader"
		${GLSL_FILES})

set(RESOURCES_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/spirv_resources")
set(RESOURCES_HEADER "${RESOURCES_SRC_DIR}/spirv_resources.h")
set(RESOURCES_SRC "${RESOURCES_SRC_DIR}/spirv_resources.cpp")
set(RESOURCES_RELATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/shader")
file(MAKE_DIRECTORY ${RESOURCES_SRC_DIR})
include_directories(${RESOURCES_SRC_DIR})
generate_resources(SETUP GENERATE_HEADER
		BINARY
		RELATIVE_PATH ${RESOURCES_RELATIVE_PATH}
		HEADER_FILE ${RESOURCES_HEADER}
		HEADER_FILE_RELATIVE_PATH ${RESOURCES_SRC_DIR}
		SOURCE_FILE ${RESOURCES_SRC}
		FUNCTION_PREFIX "spirv_resources"
		INCLUDE_GUARD "SPIRV_RESOURCES"
		FILES ${SPIRV_FILES})


add_library(lavos SHARED ${SOURCE_FILES} ${RESOURCES_HEADERS} ${RESOURCES_SRC})
target_link_libraries(lavos ${Vulkan_LIBRARIES})
target_include_directories(lavos PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/vma/src")



