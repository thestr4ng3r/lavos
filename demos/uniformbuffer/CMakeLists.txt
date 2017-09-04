
set(SOURCE_FILES
		uniformbuffer_application.cpp
		uniformbuffer_application.h)

set(GLSL_FILES
		color.vert
		color.frag)

add_demo(uniformbuffer SOURCE_FILES GLSL_FILES)