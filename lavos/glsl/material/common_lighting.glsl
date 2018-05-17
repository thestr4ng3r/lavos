
#ifndef _MATERIAL_COMMON_LIGHTING_GLSL
#define _MATERIAL_COMMON_LIGHTING_GLSL

#include "common.glsl"

#define MAX_SPOT_LIGHTS_COUNT 16

struct SpotLight
{
    vec3 position;
    float angle_cos;
    vec3 direction;
};

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = 1, std140) uniform LightingBuffer
{
	vec3 ambient_intensity;

	bool directional_light_enabled;
	vec3 directional_light_dir;
	vec3 directional_light_intensity;

	uint spot_lights_count;

	SpotLight spot_lights[MAX_SPOT_LIGHTS_COUNT];
} lighting_uni;

#endif