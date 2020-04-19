#version 410 core

// Output
layout (location = 0) out vec4 color;

void main(void)
{
    // Write final color to the framebuffer
    color = vec4(1.0, 0.5, 0.0, 1.0);
}
