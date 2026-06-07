// Stage 21: Scene Graph
// Demonstrates per-draw-call world transform via root constants.
// Each object gets its own 4x4 world matrix + flat color pushed as 20 x 32-bit constants.

#pragma pack_matrix(row_major)

cbuffer PerObject : register(b0)
{
    float4x4 WorldMatrix; // 16 constants (rows 0-3)
    float4   Color;       // 4 constants
};

struct VertexIn
{
    float2 Position : POSITION;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    // Transform from object-space unit quad to NDC via the per-object world matrix.
    output.Position = mul(float4(input.Position, 0.0f, 1.0f), WorldMatrix);
    output.Color    = Color;
    return output;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    return input.Color;
}
