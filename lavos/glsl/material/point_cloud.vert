#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "common_vert.glsl"
#include "common_lighting.glsl"
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