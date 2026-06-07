struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

Texture2D OffscreenTexture : register(t0);
SamplerState LinearClampSampler : register(s0);

VertexOut OffscreenVS(uint vertexId : SV_VertexID)
{
    static const float2 positions[3] =
    {
        float2(0.00f, 0.72f),
        float2(0.72f, -0.58f),
        float2(-0.72f, -0.58f)
    };

    static const float3 colors[3] =
    {
        float3(1.00f, 0.30f, 0.20f),
        float3(0.18f, 0.90f, 0.44f),
        float3(0.20f, 0.48f, 1.00f)
    };

    VertexOut output;
    output.Position = float4(positions[vertexId], 0.0f, 1.0f);
    output.Color = colors[vertexId];
    output.TexCoord = 0.0f;
    return output;
}

float4 OffscreenPS(VertexOut input) : SV_TARGET
{
    return float4(input.Color, 1.0f);
}

VertexOut ScreenVS(uint vertexId : SV_VertexID)
{
    static const float2 positions[6] =
    {
        float2(-1.0f, 1.0f),
        float2(1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, -1.0f)
    };

    static const float2 texCoords[6] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f)
    };

    VertexOut output;
    output.Position = float4(positions[vertexId], 0.0f, 1.0f);
    output.Color = 1.0f;
    output.TexCoord = texCoords[vertexId];
    return output;
}

float4 ScreenPS(VertexOut input) : SV_TARGET
{
    float4 sampled = OffscreenTexture.Sample(LinearClampSampler, input.TexCoord);
    return float4(sampled.rgb, 1.0f);
}
