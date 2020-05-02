
#ifndef _MATERIAL_COMMON_LIGHTING_GLSL
#define _MATERIAL_COMMON_LIGHTING_GLSL

#include "common.glsl"

struct SpotLight
{
    vec3 position;
    float angle_cos;
    vec3 direction;
    mat4 shadow_mvp_matrix;
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

#if SHADOW_MSM
#include "../lib/msm.glsl"
#endif

#define SPOT_LIGHT_SHADOW_DEPTH_BIAS 0.0001

float EvaluateSpotLightShadow(int index, vec3 pos)
{
	vec4 shadow_pos = lighting_uni.spot_lights[index].shadow_mvp_matrix * vec4(pos, 1.0);
#if SHADOW_MSM
	vec2 uv = shadow_pos.xy * 0.5 / shadow_pos.w + 0.5;
	return MSMShadow(texture(spot_light_shadow_tex_uni[index], uv), shadow_pos.z);
#else
	shadow_pos /= shadow_pos.w;
	vec2 uv = shadow_pos.xy * 0.5 + 0.5;
	float shadow_depth = texture(spot_light_shadow_tex_uni[index], uv).r;
	float frag_depth = shadow_pos.z - SPOT_LIGHT_SHADOW_DEPTH_BIAS;
	return frag_depth < shadow_depth ? 1.0 : 0.0;
#endif
}

#endif