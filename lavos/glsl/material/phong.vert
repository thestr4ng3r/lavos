#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_vert.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 uv_out;

layout(location = 1) out vec3 position_out;
layout(location = 2) out vec3 normal_out;
layout(location = 3) out vec3 tang_out;
layout(location = 4) out vec3 bitang_out;

void main()
{
	uv_out = uv_in;

	position_out = (transform_push_constant.transform * vec4(position_in, 1.0)).xyz;
	normal_out = mat3(transform_push_constant.transform) * normal_in;
	tang_out = mat3(transform_push_constant.transform) * tang_in;
	bitang_out = mat3(transform_push_constant.transform) * bitang_in;

	gl_Position = CalculateVertexPosition();
}