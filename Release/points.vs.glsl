#version 410 core

// Per-vertex inputs
layout (location = 0) in vec4 position0;
layout (location = 1) in vec4 position1;
layout (location = 2) in vec4 position2;
layout (location = 3) in vec4 position3;
layout (location = 4) in vec4 position4;
layout (location = 5) in vec4 position5;
layout (location = 6) in vec4 position6;
layout (location = 7) in vec4 position7;

out VS_OUT
{
    vec4 position0;
    vec4 position1;
    vec4 position2;
    vec4 position3;
    vec4 position4;
    vec4 position5;
    vec4 position6;
    vec4 position7;
} vs_out;

void main(void)
{
    vs_out.position0 = position0;
    vs_out.position1 = position1;
    vs_out.position2 = position2;
    vs_out.position3 = position3;
    vs_out.position4 = position4;
    vs_out.position5 = position5;
    vs_out.position6 = position6;
    vs_out.position7 = position7;

    vec4 avg_position = position0 + position1 + position2 + position3 + position4 + position5 + position6 + position7;
    avg_position /= 8.0f;

    // Calculate the clip-space position of each vertex
    gl_Position = avg_position;
}
