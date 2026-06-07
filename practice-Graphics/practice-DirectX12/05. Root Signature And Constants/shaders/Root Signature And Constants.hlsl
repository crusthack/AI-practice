// Root Signature And Constants.hlsl
//
// Each draw call supplies a fresh set of root constants describing one object:
//   gOffset  — NDC-space translation (applied after rotation and scale)
//   gScale   — uniform scale in local space
//   gAngle   — CCW rotation in radians
//   gTint    — per-object RGBA colour multiplier
//
// The vertex buffer holds a single unit equilateral triangle (all white).
// All shape, position, and colour variation is driven purely by root constants —
// no descriptor heap is touched and no buffer is rebound between draw calls.

cbuffer PerObject : register(b0)
{
    float2 gOffset;
    float  gScale;
    float  gAngle;
    float4 gTint;
};

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
    const float c = cos(gAngle);
    const float s = sin(gAngle);

    // Rotate around local origin, then scale, then translate to world position.
    float2 rotated = float2(
        c * input.position.x - s * input.position.y,
        s * input.position.x + c * input.position.y);

    PSInput output;
    output.position = float4(rotated * gScale + gOffset, 0.0f, 1.0f);
    output.color    = input.color * gTint;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
