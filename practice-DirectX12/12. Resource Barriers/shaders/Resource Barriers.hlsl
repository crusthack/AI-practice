// Resource Barriers.hlsl
//
// Offscreen pass  : procedural plasma pattern animated with time (no vertex buffer — SV_VertexID only).
// Screen pass     : full-screen blit of the offscreen texture.
//
// The offscreen texture cycles between two states every frame driven by explicit barriers in C++:
//   Barrier 1  RENDER_TARGET -> PIXEL_SHADER_RESOURCE  (after offscreen draw, before screen sample)
//   Barrier 2  PIXEL_SHADER_RESOURCE -> RENDER_TARGET  (end of frame, ready for next offscreen pass)

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 Uv       : TEXCOORD0;
};

// ---- Offscreen pass ----

cbuffer OffscreenConstants : register(b0)
{
    float Time;
    float Width;
    float Height;
};

VertexOut OffscreenVS(uint vertexId : SV_VertexID)
{
    // One large triangle that covers the entire [-1, 1] NDC box.
    // The GPU clips it to the viewport, so every pixel is covered exactly once.
    static const float2 positions[3] =
    {
        float2(-1.0f,  3.0f),
        float2( 3.0f, -1.0f),
        float2(-1.0f, -1.0f)
    };

    VertexOut o;
    o.Position = float4(positions[vertexId], 0.0f, 1.0f);
    o.Uv = float2(0.0f, 0.0f); // unused in OffscreenPS
    return o;
}

float4 OffscreenPS(VertexOut input) : SV_TARGET
{
    // Convert SV_POSITION (pixel centre in screen space) to [0, 1] UV.
    float2 uv = float2(input.Position.x / Width, input.Position.y / Height);

    // Three overlapping sine waves produce an animated plasma pattern.
    float wave = sin(uv.x * 10.0f + Time * 1.3f)
               + cos(uv.y *  8.0f + Time * 0.9f)
               + sin((uv.x + uv.y) * 6.0f + Time * 1.7f);

    // Offset R/G/B by 120 degrees (2pi/3) for full colour cycling.
    return float4(
        0.5f + 0.5f * sin(wave + Time),
        0.5f + 0.5f * sin(wave + Time + 2.094f),
        0.5f + 0.5f * sin(wave + Time + 4.189f),
        1.0f);
}

// ---- Screen pass ----

Texture2D   OffscreenTexture     : register(t0);
SamplerState LinearClampSampler  : register(s0);

VertexOut ScreenVS(uint vertexId : SV_VertexID)
{
    // Two triangles forming a full-screen quad.
    static const float2 positions[6] =
    {
        float2(-1.0f,  1.0f), float2( 1.0f,  1.0f), float2(-1.0f, -1.0f),
        float2(-1.0f, -1.0f), float2( 1.0f,  1.0f), float2( 1.0f, -1.0f)
    };
    static const float2 uvs[6] =
    {
        float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f),
        float2(0.0f, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 1.0f)
    };

    VertexOut o;
    o.Position = float4(positions[vertexId], 0.0f, 1.0f);
    o.Uv = uvs[vertexId];
    return o;
}

float4 ScreenPS(VertexOut input) : SV_TARGET
{
    return OffscreenTexture.Sample(LinearClampSampler, input.Uv);
}
