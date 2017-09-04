#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform MatrixBuffer
{
	mat4 model;
	mat4 view;
	mat4 projection;
} matrix_uni;


layout(location = 0) in vec2 position_in;
layout(location = 1) in vec3 color_in;


out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 frag_color_out;


void main()
{
	gl_Position = matrix_uni.projection * matrix_uni.view * matrix_uni.model * vec4(position_in, 0.0, 1.0);
	frag_color_out = color_in;
}