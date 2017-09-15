
layout(set = 0, binding = 0) uniform MatrixBuffer
{
	mat4 modelview;
	mat4 projection;
} matrix_uni;

layout(push_constant) uniform TransformPushConstant
{
	mat4 transform;
} transform_push_constant;

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 color_in;
layout(location = 2) in vec2 uv_in;

vec4 CalculateVertexPosition()
{
	return matrix_uni.projection
		* matrix_uni.modelview
		* transform_push_constant.transform
		* vec4(position_in, 1.0);
}