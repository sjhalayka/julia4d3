#version 410 core


// Per-vertex inputs
layout (location = 0) in vec4 position;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void)
{
    // Calculate view-space coordinate
    vec4 P = mv_matrix * position;

    // Calculate the clip-space position of each vertex
    gl_Position = proj_matrix * P;
}
