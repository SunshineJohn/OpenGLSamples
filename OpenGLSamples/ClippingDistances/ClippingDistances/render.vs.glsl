#version 450 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
/*
layout (std140) uniform constants
{
	mat4 mv_matrix;
	mat4 view_matrix;
	mat4 proj_matrix;
};
*/
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

out VS_OUT
{
	vec3 N;
	vec3 L;
	vec3 V;
} vs_out;

uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

uniform vec4 clip_plane = vec4(1.0, 1.0, 0.0, 0.85);
uniform vec4 clip_sphere = vec4(0.0, 0.0, 0.0, 4.0);

void main(void)
{
	vec4 P = mv_matrix * position;
	vs_out.N = mat3(mv_matrix) * normal;

	vs_out.L = light_pos - P.xyz;

	vs_out.V = -P.xyz;

	gl_ClipDistance[0] = dot(position, clip_plane);
	gl_ClipDistance[1] = length(position.xyz/position.w - clip_sphere.xyz) - clip_sphere.w;

	gl_Position = proj_matrix * P; 
}