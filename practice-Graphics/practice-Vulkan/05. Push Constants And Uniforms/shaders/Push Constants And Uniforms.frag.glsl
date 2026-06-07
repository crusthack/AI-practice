#version 450

layout(push_constant) uniform PushTint
{
    vec4 tint;
} uPush;

layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vColor, 1.0) * uPush.tint;
}
