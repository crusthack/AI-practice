// Stage 25: Post Processing
// Two-pass rendering:
//   Pass 1 (SceneVS / ScenePS): render 4 colored rotating quads to an offscreen texture.
//     Uses the same world-matrix + flat-color approach as Stage 21 Scene Graph.
//   Pass 2 (PostVS / PostPS): fullscreen triangle that samples the offscreen texture and applies:
//     - Grayscale conversion (luminance weights)
//     - Color-fade: lerp between grayscale and original color based on an animated factor
//     - Vignette: darken edges using smoothstep on distance from centre

#pragma pack_matrix(row_major)

// ============================================================
// Scene pass
// ============================================================

cbuffer PerObject : register(b0)
{
    float4x4 WorldMatrix; // 16 floats (offsets 0-15)
    float4   Color;       // 4 floats  (offsets 16-19)
};

struct SceneVIn
{
    float2 Position : POSITION;
};

struct SceneVOut
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

SceneVOut SceneVS(SceneVIn input)
{
    SceneVOut output;
    output.Position = mul(float4(input.Position, 0.0f, 1.0f), WorldMatrix);
    output.Color    = Color;
    return output;
}

float4 ScenePS(SceneVOut input) : SV_TARGET
{
    return input.Color;
}

// ============================================================
// Post-processing pass
// ============================================================

Texture2D    OffscreenTexture : register(t0);
SamplerState LinearSampler    : register(s0);

cbuffer PostConstants : register(b0)
{
    float ColorFade;         // 0 = full grayscale, 1 = full color
    float VignetteInner;     // distance at which vignette starts
    float VignetteOuter;     // distance at which vignette is fully dark
    float VignetteStrength;  // maximum darkness of the vignette (0..1)
};

struct PostVOut
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
};

// Generate a fullscreen triangle using vertex ID (no vertex buffer required).
// Vertex 0: (-1, +3)   UV (0, -1)
// Vertex 1: (-1, -1)   UV (0,  1)
// Vertex 2: (+3, -1)   UV (2,  1)
PostVOut PostVS(uint vertexId : SV_VertexID)
{
    float2 positions[3] = {
        float2(-1.0f,  3.0f),
        float2(-1.0f, -1.0f),
        float2( 3.0f, -1.0f),
    };
    float2 uvs[3] = {
        float2(0.0f, -1.0f),
        float2(0.0f,  1.0f),
        float2(2.0f,  1.0f),
    };

    PostVOut output;
    output.Position = float4(positions[vertexId], 0.0f, 1.0f);
    output.UV       = uvs[vertexId];
    return output;
}

float4 PostPS(PostVOut input) : SV_TARGET
{
    float4 color = OffscreenTexture.Sample(LinearSampler, input.UV);

    // Grayscale using perceptual luminance weights.
    float luma = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    float3 gray = float3(luma, luma, luma);

    // Blend between grayscale and original color based on ColorFade.
    float3 blended = lerp(gray, color.rgb, ColorFade);

    // Vignette: darkens edges based on distance from screen centre.
    // UV centre is (0.5, 0.5); compute distance and map to [0,1] darkness.
    float2 centred = input.UV - float2(0.5f, 0.5f);
    float  dist    = length(centred) * 2.0f;  // scale so corners ~= 1.414
    float  vignette = smoothstep(VignetteInner, VignetteOuter, dist);
    float  darkening = 1.0f - vignette * VignetteStrength;

    return float4(blended * darkening, 1.0f);
}
