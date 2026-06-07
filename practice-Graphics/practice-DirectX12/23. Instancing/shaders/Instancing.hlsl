// Stage 23: Instancing
// Draws a 10x10 grid of quads using two vertex buffer streams and a single DrawIndexedInstanced call.
// Stream 0 (PER_VERTEX):   float2 Position         — unit quad corners in object space.
// Stream 1 (PER_INSTANCE): float2 InstanceOffset   — NDC offset for each grid cell.
//                          float4 InstanceColor    — RGBA color for this cell.
// Root constant b0: QuadScale — animated half-size of each quad in NDC.

cbuffer ScaleConstants : register(b0)
{
    float QuadScale;
};

struct VertexIn
{
    float2 Position      : POSITION;        // per-vertex: unit quad corner (-1..+1)
    float2 InstanceOffset : INSTANCE_OFFSET; // per-instance: grid position in NDC
    float4 InstanceColor  : INSTANCE_COLOR;  // per-instance: RGBA color
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    // Scale the unit-quad corner by QuadScale, then shift to the instance grid position.
    float2 worldPos = input.Position * QuadScale + input.InstanceOffset;
    output.Position = float4(worldPos, 0.0f, 1.0f);
    output.Color    = input.InstanceColor;
    return output;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    return input.Color;
}
