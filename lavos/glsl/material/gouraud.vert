#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_vert.glsl"
#include "common_lighting.glsl"
#include "common_camera.glsl"
#include "lighting_phong.glsl"

layout(set = DESCRIPTOR_SET_INDEX_MATERIAL, binding = 0, std140) uniform MaterialBuffer
{
	PhongMaterialParameters phong_params;
} material_uni;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 1) out vec2 uv_out;
layout(location = 2) out vec3 color_out;

void main()
{
	gl_Position = CalculateVertexPosition();

	vec3 base_color = material_uni.phong_params.base_color.rgb;
	vec3 color = base_color.rgb * lighting_uni.ambient_intensity;

	vec3 cam_dir = normalize(camera_uni.position - position_in);

	vec3 normal = normalize(normal_in);

	if(lighting_uni.directional_light_enabled)
	{
		color += base_color.rgb * LightingPhong(normal, -lighting_uni.directional_light_dir, cam_dir, material_uni.phong_params.specular_exponent);
	}

	color_out = color;
	uv_out = uv_in;
}
