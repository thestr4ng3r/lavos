#version 450
#extension GL_ARB_separate_shader_objects : enable

#define COMMON_VERT_MATRIX_COMPACT
#include "common_vert.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = CalculateVertexPosition();
}