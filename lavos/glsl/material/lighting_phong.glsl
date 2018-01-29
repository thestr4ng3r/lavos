
#ifndef _MATERIAL_LIGHTING_PHONG_GLSL
#define _MATERIAL_LIGHTING_PHONG_GLSL

struct PhongMaterialParameters
{
	vec4 base_color;
	float specular_exponent;
};

float LightingPhong(vec3 normal, vec3 light_dir, vec3 cam_dir, float specular_exponent)
{
	float lambert = max(0.0, dot(light_dir, normal));

	float specular = max(0.0, dot(reflect(-light_dir, normal), cam_dir));
	specular = pow(specular, specular_exponent);

	return lambert + specular;
}

#endif