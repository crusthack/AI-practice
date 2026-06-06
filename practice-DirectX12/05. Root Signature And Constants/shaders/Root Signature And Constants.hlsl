cbuffer RootConstants : register(b0)
{
    float2 gOffset;
    float gPulse;
    float gPadding;
    float4 gTint;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position.xy + gOffset, input.position.z, 1.0f);
    output.color = input.color * gTint;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(input.color.rgb * gPulse, input.color.a);
}
