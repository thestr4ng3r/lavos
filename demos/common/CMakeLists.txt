
set(SOURCE_FILES
		demo_application.cpp
		demo_application.h
		shader_load.cpp
		shader_load.h)

add_library(demo_common STATIC ${SOURCE_FILES})