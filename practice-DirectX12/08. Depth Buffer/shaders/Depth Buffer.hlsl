// Depth Buffer.hlsl
//
// gTime drives opposite-phase Z oscillation for the two triangles.
// The orange triangle and the blue triangle periodically swap depth order,
// making the depth test visually apparent without relying on static geometry.

cbuffer AnimConstants : register(b0) { float gTime; };

struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    // Blue triangle has base Z > 0.5 (originally 0.65).
    // Orange triangle has base Z < 0.5 (originally 0.25).
    // Each oscillates around 0.5 in opposite phases so they cross each other.
    float phase = (input.position.z > 0.5f) ? 1.0f : -1.0f;
    float animZ = 0.5f + phase * sin(gTime * 1.2f) * 0.35f;

    PSInput output;
    output.position = float4(input.position.xy, animZ, 1.0f);
    output.color    = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
