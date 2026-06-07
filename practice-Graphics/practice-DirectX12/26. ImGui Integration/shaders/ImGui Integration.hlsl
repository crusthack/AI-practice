// Stage 26: ImGui Integration
// Demonstrates the DX12 descriptor heap infrastructure that Dear ImGui relies on,
// and renders a manual debug overlay using alpha-blended screen-space quads.
//
// Four entry points:
//   BackgroundVS / BackgroundPS  — world-matrix root-constants (same pattern as stage 21)
//   OverlayVS    / OverlayPS     — NDC-rect + RGBA root constants for the UI panels

#pragma pack_matrix(row_major)

// ---------------------------------------------------------------------------
// Background pass — root constants at b0: float4x4 WorldMatrix + float4 Color
// ---------------------------------------------------------------------------
cbuffer BackgroundConstants : register(b0)
{
    float4x4 WorldMatrix;   // 16 x 32-bit constants
    float4   Color;         // 4  x 32-bit constants  (total 20)
};

struct BackgroundVertexIn
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct BackgroundVertexOut
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

BackgroundVertexOut BackgroundVS(BackgroundVertexIn input)
{
    BackgroundVertexOut o;
    // Transform from object-space unit quad through world matrix into NDC.
    o.Position = mul(float4(input.Position, 0.0f, 1.0f), WorldMatrix);
    o.Color    = Color;
    return o;
}

float4 BackgroundPS(BackgroundVertexOut input) : SV_TARGET
{
    return input.Color;
}

// ---------------------------------------------------------------------------
// Overlay pass — root constants at b0:
//   float4 Rect  = (x, y, w, h) in NDC  — 4 x 32-bit
//   float4 RGBA                          — 4 x 32-bit  (total 8)
// ---------------------------------------------------------------------------
cbuffer OverlayConstants : register(b0)
{
    float4 Rect;    // x=left, y=top (NDC), z=width, w=height (NDC extents)
    float4 RGBA;    // panel color including alpha
};

struct OverlayVertexOut
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

// The overlay VS generates its quad from SV_VertexID (no vertex buffer needed).
OverlayVertexOut OverlayVS(uint vid : SV_VertexID)
{
    // Two triangles: indices 0-5 form a quad
    // Vertex order (row-major, two triangles):
    //   0: top-left    1: top-right    2: bottom-left
    //   3: top-right   4: bottom-right 5: bottom-left
    float2 uv;
    uv.x = (vid == 1 || vid == 3 || vid == 4) ? 1.0f : 0.0f;
    uv.y = (vid == 2 || vid == 4 || vid == 5) ? 1.0f : 0.0f;

    float ndcX = Rect.x + uv.x * Rect.z;
    float ndcY = Rect.y - uv.y * Rect.w;   // NDC Y grows upward; panel H grows downward

    OverlayVertexOut o;
    o.Position = float4(ndcX, ndcY, 0.0f, 1.0f);
    o.Color    = RGBA;
    return o;
}

float4 OverlayPS(OverlayVertexOut input) : SV_TARGET
{
    return input.Color;
}
