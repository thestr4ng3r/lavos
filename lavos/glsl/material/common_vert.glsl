
#ifndef _MATERIAL_COMMON_VERT_H
#define _MATERIAL_COMMON_VERT_H

#include "common.glsl"

layout(set = DESCRIPTOR_SET_INDEX_COMMON, binding = 0) uniform MatrixBuffer
{
#ifdef COMMON_VERT_MATRIX_COMPACT
    mat4 modelview_projection;
#else
	mat4 modelview;
	mat4 projection;
#endif
} matrix_uni;

layout(push_constant) uniform TransformPushConstant
{
	mat4 transform;
} transform_push_constant;

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec3 tang_in;
layout(location = 4) in vec3 bitang_in;

vec4 CalculateVertexPosition()
{
	return
#ifdef COMMON_VERT_MATRIX_COMPACT
		matrix_uni.modelview_projection
#else
		matrix_uni.projection
		* matrix_uni.modelview
#endif
		* transform_push_constant.transform
		* vec4(position_in, 1.0);
}

#endif