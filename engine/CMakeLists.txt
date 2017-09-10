

set(SOURCE_FILES
		src/thirdparty_impl.cpp
		src/engine.cpp
		include/engine.h
		include/vertex.h
		src/mesh.cpp
		include/mesh.h
		src/material.cpp
		include/material.h
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
		include/scene.h)

add_library(engine STATIC ${SOURCE_FILES})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(engine ${Vulkan_LIBRARIES})