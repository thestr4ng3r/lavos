
#ifndef _MATERIAL_COMMON_LIGHTING_GLSL
#define _MATERIAL_COMMON_LIGHTING_GLSL

#include "common.glsl"

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = 1, std140) uniform LightingBuffer
{
	bool directional_light_enabled;
	vec3 directional_light_dir;
	vec3 directional_light_intensity;
} lighting_uni;

#endif