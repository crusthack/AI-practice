#version 450

layout(binding = 0) uniform PerFrame
{
    vec2 offset;
} uFrame;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vColor;

void main()
{
    gl_Position = vec4(inPosition + uFrame.offset, 0.0, 1.0);
    vColor = inColor;
}
