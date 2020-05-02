#version 450

#ifdef SHADER_VERT

#define COMMON_VERT_MATRIX_COMPACT
#include "common_vert.glsl"

#if SHADOW_MSM
layout(location = 0) out float z_out;
#endif

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec4 pos = CalculateVertexPosition();
	#if SHADOW_MSM
		z_out = pos.z;
	#endif
	gl_Position = pos;
}

// -------------------------------------------------
#elif defined(SHADER_FRAG)

#if SHADOW_MSM
layout(location = 0) in float z_in;
layout(location = 0) out vec4 shadow_out;
#endif

void main()
{
	#if SHADOW_MSM
		float z = z_in;
		shadow_out = vec4(z, z*z, z*z*z, z*z*z*z);
	#endif
}

#endif