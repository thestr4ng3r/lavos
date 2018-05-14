#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_frag.glsl"

layout(location = 1) in vec3 color_in;

void main()
{
	out_color = vec4(color_in, 1.0);
}