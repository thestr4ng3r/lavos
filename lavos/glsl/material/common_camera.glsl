
#ifndef _MATERIAL_COMMON_CAMERA_GLSL
#define _MATERIAL_COMMON_CAMERA_GLSL

#include "common.glsl"

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = 2, std140) uniform CameraBuffer
{
	vec3 position;
} camera_uni;

#endif