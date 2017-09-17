#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_frag.glsl"
#include "common_lighting.glsl"

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 0) uniform sampler2D tex_uni;

layout(location = 0) in vec3 frag_color_in;
layout(location = 1) in vec2 uv_in;

void main()
{
	out_color = texture(tex_uni, uv_in).rgba;
}