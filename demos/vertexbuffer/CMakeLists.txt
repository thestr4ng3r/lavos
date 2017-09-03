
set(SOURCE_FILES
		vertexbuffer_application.cpp
		vertexbuffer_application.h)

set(GLSL_FILES
		color.vert
		color.frag)

add_demo(vertexbuffer SOURCE_FILES GLSL_FILES)