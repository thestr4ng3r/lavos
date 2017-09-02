

set(SOURCE_FILES
		src/engine.cpp
		include/engine.h)

add_library(engine STATIC ${SOURCE_FILES})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(engine ${Vulkan_LIBRARIES})