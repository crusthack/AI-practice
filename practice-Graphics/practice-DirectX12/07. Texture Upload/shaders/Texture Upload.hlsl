// Texture Upload.hlsl
//
// A 1-DWORD root constant (gTime) drives UV scrolling in the vertex shader.
// The texture itself never changes — only the UV coordinate shifts each frame,
// demonstrating that animation does not require re-uploading texture data.

cbuffer AnimConstants : register(b0) { float gTime; };

Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0f);
    // Tile 2× and scroll horizontally — WRAP addressing makes the loop seamless.
    output.uv = float2(input.uv.x * 2.0f + gTime * 0.06f, input.uv.y * 2.0f);
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return gTexture.Sample(gSampler, input.uv);
}
