#version 450 core

layout (location = 0) in vec4 position_3;
layout (location = 1) in vec3 normal;

layout (location = 10) in uint draw_id;

out VS_OUT
{
	vec3 normal;
	vec4 color;
} vs_out;

uniform float time = 0.0;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 viewproj_matrix;

const vec4 color0 = vec4(0.29, 0.21, 0.18, 1.0);
const vec4 color1 = vec3(0.58, 0.55, 0.51, 1.0);

void main(void)
{
	vec4 position = vec4(position3, 1.0);

	mat4 m1;
	mat4 m2;
	mat4 m;
	float t = time * 0.1;
}