#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common.glsl"

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = 0) uniform MatrixBuffer
{
	mat4 modelview;
	mat4 projection;
} matrix_uni;

layout(push_constant) uniform TransformPushConstant
{
	mat4 transform;
} transform_push_constant;

layout(location = 0) in vec3 position_in;

vec4 CalculateVertexPosition()
{
	return matrix_uni.projection
		* matrix_uni.modelview
		* transform_push_constant.transform
		* vec4(position_in, 1.0);
}

#include "common_camera.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

layout(location = 1) out vec3 color_out;

void main()
{
	gl_Position = CalculateVertexPosition();
	gl_PointSize = 1.0f;
	color_out = vec3(1.0);
}