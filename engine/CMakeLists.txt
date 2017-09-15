

set(GLSLC_EXECUTABLE glslc) # TODO: Write a find module or something

macro(add_spirv_shaders SPIRV_FILES_VAR GLSL_FILES_VAR)
	set(${SPIRV_FILES_VAR} "")
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shader")
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
		src/material_instance.cpp
		include/material_instance.h
		include/buffer.h
		src/image.cpp
		include/image.h
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
		src/glm_stream.h)

set(GLSL_FILES
		unlit.vert
		unlit.frag)

add_spirv_shaders(SPIRV_FILES GLSL_FILES)

add_library(engine STATIC ${SOURCE_FILES} ${SPIRV_FILES})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(engine ${Vulkan_LIBRARIES})