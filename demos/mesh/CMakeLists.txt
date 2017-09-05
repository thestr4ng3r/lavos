
set(SOURCE_FILES
		mesh_application.cpp
		mesh_application.h)

set(GLSL_FILES
		texture.vert
		texture.frag)

add_demo(mesh SOURCE_FILES GLSL_FILES)