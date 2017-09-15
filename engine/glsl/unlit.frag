#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D tex_uni;

layout(location = 0) in vec3 frag_color_in;
layout(location = 1) in vec2 uv_in;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = texture(tex_uni, uv_in);
}