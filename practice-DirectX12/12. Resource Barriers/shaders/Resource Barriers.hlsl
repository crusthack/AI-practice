Texture2D BarrierTexture : register(t0);
SamplerState PointSampler : register(s0);

struct VSInput
{
    float3 Position : POSITION;
    float2 Uv : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0f);
    output.Uv = input.Uv;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return BarrierTexture.Sample(PointSampler, input.Uv);
}
