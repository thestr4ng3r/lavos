
set(SOURCE_FILES
		demo_application.cpp
		demo_application.h
		../../engine/src/shader_load.cpp
		../../engine/include/shader_load.h)

add_library(demo_common STATIC ${SOURCE_FILES})