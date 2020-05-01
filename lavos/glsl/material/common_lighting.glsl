
#ifndef _MATERIAL_COMMON_LIGHTING_GLSL
#define _MATERIAL_COMMON_LIGHTING_GLSL

#include "common.glsl"

struct SpotLight
{
    vec3 position;
    float angle_cos;
    vec3 direction;
};

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = DESCRIPTOR_SET_COMMON_BINDING_LIGHTING_BUFFER, std140) uniform LightingBuffer
{
	vec3 ambient_intensity;

	bool directional_light_enabled;
	vec3 directional_light_dir;
	vec3 directional_light_intensity;

	uint spot_lights_count;

	SpotLight spot_lights[MAX_SPOT_LIGHTS_COUNT];
} lighting_uni;

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = DESCRIPTOR_SET_COMMON_BINDING_SPOT_LIGHT_SHADOW_TEX) uniform sampler2D spot_light_shadow_tex_uni[MAX_SPOT_LIGHTS_COUNT];

#endif