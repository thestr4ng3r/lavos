#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_vert.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 1) out vec2 uv_out;

void main()
{
	gl_Position = CalculateVertexPosition();

	uv_out = uv_in;
}