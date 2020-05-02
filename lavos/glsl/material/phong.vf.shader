#version 450

#if SHADER_VERT

#include "common_vert.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 uv_out;

layout(location = 1) out vec3 position_out;
layout(location = 2) out vec3 normal_out;
layout(location = 3) out vec3 tang_out;
layout(location = 4) out vec3 bitang_out;

void main()
{
	uv_out = uv_in;

	position_out = (transform_push_constant.transform * vec4(position_in, 1.0)).xyz;
	normal_out = mat3(transform_push_constant.transform) * normal_in;
	tang_out = mat3(transform_push_constant.transform) * tang_in;
	bitang_out = mat3(transform_push_constant.transform) * bitang_in;

	gl_Position = CalculateVertexPosition();
}

#elif SHADER_FRAG

#include "common_frag.glsl"
#include "common_lighting.glsl"
#include "common_camera.glsl"
#include "lighting_phong.glsl"

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 0, std140) uniform MaterialBuffer
{
	PhongMaterialParameters phong_params;
} material_uni;

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 1) uniform sampler2D base_color_tex_uni;
layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 2) uniform sampler2D normal_tex_uni;


layout(location = 0) in vec2 uv_in;

layout(location = 1) in vec3 position_in;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec3 tang_in;
layout(location = 4) in vec3 bitang_in;



void main()
{
	vec4 base_color = texture(base_color_tex_uni, uv_in).rgba;
	base_color *= material_uni.phong_params.base_color;

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
		color += base_color.rgb * LightingPhong(normal, -lighting_uni.directional_light_dir, cam_dir, material_uni.phong_params.specular_exponent);
	}

	for(int i=0; i<lighting_uni.spot_lights_count; i++)
	{
	    SpotLight spot = lighting_uni.spot_lights[i];
	    vec3 light_dir = normalize(spot.position - position_in);

	    float ndotl = -dot(spot.direction, light_dir);
	    if(ndotl < spot.angle_cos)
	        continue;

		float shadow = EvaluateSpotLightShadow(i, position_in);
	    color += base_color.rgb * shadow * LightingPhong(normal, light_dir, cam_dir, material_uni.phong_params.specular_exponent);
	}

	out_color = vec4(color, base_color.a);
}

#endif