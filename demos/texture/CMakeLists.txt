
set(SOURCE_FILES
		texture_application.cpp
		texture_application.h)

set(GLSL_FILES
		texture.vert
		texture.frag)

add_demo(texture SOURCE_FILES GLSL_FILES)