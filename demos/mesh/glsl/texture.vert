#version 450
#extension GL_ARB_separate_shader_objects : enable

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


out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 frag_color_out;
layout(location = 1) out vec2 uv_out;


void main()
{
	gl_Position =
		matrix_uni.projection
		* matrix_uni.modelview
		* transform_push_constant.transform
		* vec4(position_in, 1.0);

	uv_out = uv_in;
	frag_color_out = color_in;
}