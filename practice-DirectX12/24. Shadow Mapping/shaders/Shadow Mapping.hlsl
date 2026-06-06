// Stage 24: Shadow Mapping
// Two-pass shadow mapping.
//   Pass 1 (ShadowVS): transform world-space geometry to light clip-space to write depth.
//   Pass 2 (SceneVS / ScenePS): render scene with shadow test via SampleCmpLevelZero.
//
// Vertex layout: float3 Position, float3 Color (world-space, set by the CPU each frame).
//
// Root signature:
//   Shadow pass — b0: 16 root constants (LightViewProj 4x4)
//   Scene  pass — b0: 32 root constants (CameraViewProj 4x4 + LightViewProj 4x4), t0: shadow SRV

#pragma pack_matrix(row_major)

// ============================================================
// Shadow pass
// ============================================================

cbuffer ShadowConstants : register(b0)
{
    float4x4 LightViewProj; // 16 floats
};

struct ShadowVIn
{
    float3 Position : POSITION;
    float3 Color    : COLOR;   // unused in shadow pass
};

float4 ShadowVS(ShadowVIn input) : SV_POSITION
{
    return mul(float4(input.Position, 1.0f), LightViewProj);
}

// No pixel shader for the shadow pass (depth only).

// ============================================================
// Scene pass
// ============================================================

cbuffer SceneConstants : register(b0)
{
    float4x4 CameraViewProj;  // floats 0-15
    float4x4 SceneLightViewProj;   // floats 16-31
};

Texture2D<float>         ShadowMap   : register(t0);
SamplerComparisonState   ShadowSamp  : register(s0);

struct SceneVIn
{
    float3 Position : POSITION;
    float3 Color    : COLOR;
};

struct SceneVOut
{
    float4 Position     : SV_POSITION;
    float3 Color        : COLOR;
    float4 LightSpacePos : TEXCOORD0;  // position in light clip space
};

SceneVOut SceneVS(SceneVIn input)
{
    SceneVOut output;
    output.Position     = mul(float4(input.Position, 1.0f), CameraViewProj);
    output.Color        = input.Color;
    output.LightSpacePos = mul(float4(input.Position, 1.0f), SceneLightViewProj);
    return output;
}

float4 ScenePS(SceneVOut input) : SV_TARGET
{
    // Convert from clip space [-1,1] to texture UV [0,1].
    float3 lsPos = input.LightSpacePos.xyz / input.LightSpacePos.w;
    float2 shadowUV = lsPos.xy * float2(0.5f, -0.5f) + 0.5f;
    float  depth    = lsPos.z;

    // PCF: sample with a small 2x2 kernel offset for softer shadows.
    float shadow = 0.0f;
    const float texelSize = 1.0f / 512.0f;
    [unroll]
    for (int dy = -1; dy <= 1; ++dy)
    {
        [unroll]
        for (int dx = -1; dx <= 1; ++dx)
        {
            float2 offset = float2(dx, dy) * texelSize;
            shadow += ShadowMap.SampleCmpLevelZero(ShadowSamp, shadowUV + offset, depth - 0.001f);
        }
    }
    shadow /= 9.0f;

    // Shadow factor: 0 = fully shadowed, 1 = fully lit.
    float ambient = 0.25f;
    float lighting = ambient + (1.0f - ambient) * shadow;

    return float4(input.Color * lighting, 1.0f);
}
