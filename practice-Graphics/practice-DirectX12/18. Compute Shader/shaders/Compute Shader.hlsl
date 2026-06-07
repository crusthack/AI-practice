RWTexture2D<float4> OutputTexture : register(u0);
Texture2D ComputeTexture : register(t0);
SamplerState LinearClampSampler : register(s0);

cbuffer ComputeConstants : register(b0)
{
    float TimeSeconds;
    float Width;
    float Height;
    float Padding;
};

[numthreads(16, 16, 1)]
void ComputeMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x >= (uint)Width || dispatchThreadId.y >= (uint)Height)
    {
        return;
    }

    float2 uv = (float2(dispatchThreadId.xy) + 0.5f) / float2(Width, Height);
    float waves = 0.5f + 0.5f * sin((uv.x * 18.0f) + (uv.y * 11.0f) + TimeSeconds * 1.8f);
    float ring = smoothstep(0.38f, 0.18f, distance(uv, float2(0.5f, 0.5f)));

    float3 baseColor = lerp(float3(0.05f, 0.14f, 0.22f), float3(0.92f, 0.35f, 0.12f), waves);
    float3 highlight = float3(0.12f, 0.86f, 0.68f) * ring;
    OutputTexture[dispatchThreadId.xy] = float4(saturate(baseColor + highlight), 1.0f);
}

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

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
    output.TexCoord = texCoords[vertexId];
    return output;
}

float4 ScreenPS(VertexOut input) : SV_TARGET
{
    return ComputeTexture.Sample(LinearClampSampler, input.TexCoord);
}
