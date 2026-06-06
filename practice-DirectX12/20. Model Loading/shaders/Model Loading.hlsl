struct VertexIn
{
    float3 Position : POSITION;
    float3 Color : COLOR0;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR0;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    return float4(input.Color, 1.0f);
}
