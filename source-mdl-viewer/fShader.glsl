#version 430
layout (location=0) in vec3 position;

out vec4 color;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void)
{
	color = vec4(1.0, 0.0, 0.0, 1.0);
}