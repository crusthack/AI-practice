#version 450

layout(binding = 1) uniform Tint
{
    vec4 tint;
} uTint;

layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vColor, 1.0) * uTint.tint;
}
