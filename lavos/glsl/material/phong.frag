#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_frag.glsl"
#include "common_lighting.glsl"
#include "common_camera.glsl"


layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 0, std140) uniform MaterialBuffer
{
	vec4 base_color_factor;
	float specular_exponent;
} material_uni;

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 1) uniform sampler2D base_color_tex_uni;
layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 2) uniform sampler2D normal_tex_uni;


layout(location = 0) in vec2 uv_in;

layout(location = 1) in vec3 position_in;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec3 tang_in;
layout(location = 4) in vec3 bitang_in;


float LightingPhong(vec3 normal, vec3 light_dir, vec3 cam_dir)
{
	float lambert = max(0.0, dot(light_dir, normal));

	float specular = max(0.0, dot(reflect(-light_dir, normal), cam_dir));
	specular = pow(specular, material_uni.specular_exponent);

	return lambert + specular;
}

void main()
{
	vec4 base_color = texture(base_color_tex_uni, uv_in).rgba;
	base_color *= material_uni.base_color_factor;

	vec3 normal = normalize(normal_in);
	vec3 tang = normalize(tang_in);
	vec3 bitang = normalize(bitang_in);

	vec3 tang_normal = texture(normal_tex_uni, uv_in).rgb * 2.0 - 1.0;
	normal = mat3(tang, bitang, normal) * tang_normal;
	normal = normalize(normal);

	vec3 cam_dir = normalize(camera_uni.position - position_in);

	vec3 color = base_color.rgb * lighting_uni.ambient_intensity;

	if(lighting_uni.directional_light_enabled)
	{
		color += base_color.rgb * LightingPhong(normal, -lighting_uni.directional_light_dir, cam_dir);
	}

	out_color = vec4(color, base_color.a);
}