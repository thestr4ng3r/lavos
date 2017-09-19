#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_frag.glsl"
#include "common_lighting.glsl"


layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 0, std140) uniform MaterialBuffer
{
	vec3 base_color_factor;
} material_uni;

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 1) uniform sampler2D tex_uni;


layout(location = 0) in vec2 uv_in;
layout(location = 1) in vec3 normal_in;


void main()
{
	vec4 base_color = texture(tex_uni, uv_in).rgba;
	base_color.rgb *= material_uni.base_color_factor;

	vec3 normal = normalize(normal_in);

	vec3 color = base_color.rgb * lighting_uni.ambient_intensity;

	if(lighting_uni.directional_light_enabled)
	{
		float lambert = max(0.0, dot(-lighting_uni.directional_light_dir, normal));
		color += base_color.rgb * lambert;
	}

	out_color = vec4(color, base_color.a);
}