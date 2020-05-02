#version 450

#ifdef SHADER_VERT

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

#elif defined(SHADER_FRAG)

void main()
{
}

#endif