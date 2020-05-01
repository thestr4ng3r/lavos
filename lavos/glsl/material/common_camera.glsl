
#ifndef _MATERIAL_COMMON_CAMERA_GLSL
#define _MATERIAL_COMMON_CAMERA_GLSL

#include "common.glsl"

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = DESCRIPTOR_SET_COMMON_BINDING_CAMERA_BUFFER, std140) uniform CameraBuffer
{
	vec3 position;
} camera_uni;

#endif