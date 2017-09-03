
set(SOURCE_FILES
		indexbuffer_application.cpp
		indexbuffer_application.h)

set(GLSL_FILES
		color.vert
		color.frag)

add_demo(indexbuffer SOURCE_FILES GLSL_FILES)