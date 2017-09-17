#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_vert.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 uv_out;

layout(location = 1) out vec3 normal_out;

void main()
{
	uv_out = uv_in;
	normal_out = normal_in;

	gl_Position = CalculateVertexPosition();
}