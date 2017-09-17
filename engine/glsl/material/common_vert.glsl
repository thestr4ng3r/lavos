
#ifndef _MATERIAL_COMMON_VERT_H
#define _MATERIAL_COMMON_VERT_H

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
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 normal_in;

vec4 CalculateVertexPosition()
{
	return matrix_uni.projection
		* matrix_uni.modelview
		* transform_push_constant.transform
		* vec4(position_in, 1.0);
}

#endif