#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_frag.glsl"

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 1) uniform sampler2D tex_uni;

layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 color_in;

void main()
{
	vec4 tex_color = texture(tex_uni, uv_in);
	out_color = vec4(tex_color.rgb * color_in, tex_color.a);
}