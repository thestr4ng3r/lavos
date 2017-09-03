
set(SOURCE_FILES
		triangle_application.cpp
		triangle_application.h)

set(GLSL_FILES
		triangle.vert
		triangle.frag)

add_demo(triangle SOURCE_FILES GLSL_FILES)