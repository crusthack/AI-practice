// Frame Resources.hlsl
//
// Each of the two frames-in-flight has its own copy of FrameConstants.
// The CPU writes the current time into whichever slot the GPU is not currently
// reading, so smooth rotation is safe without a GPU stall.

cbuffer FrameConstants : register(b0)
{
    float  Angle;    // rotation angle in radians, different per frame slot
    float  Scale;
    float2 Offset;
    float4 Tint;
};

struct VSInput
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    const float c = cos(Angle);
    const float s = sin(Angle);
    float2 pos = input.Position.xy;
    float2 rotated = float2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);

    PSInput output;
    output.Position = float4(rotated * Scale + Offset, input.Position.z, 1.0f);
    output.Color    = input.Color * Tint;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.Color;
}
