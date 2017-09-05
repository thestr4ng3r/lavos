
set(SOURCE_FILES
		depthbuffer_application.cpp
		depthbuffer_application.h)

set(GLSL_FILES
		texture.vert
		texture.frag)

add_demo(depthbuffer SOURCE_FILES GLSL_FILES)