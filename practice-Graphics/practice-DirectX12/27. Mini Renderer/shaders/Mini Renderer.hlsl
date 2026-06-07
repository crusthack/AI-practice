// Stage 27: Mini Renderer — Capstone
// Combines: multi-object scene graph, per-pixel Phong lighting, animated camera, RTT + post-process.
//
// Four entry points:
//   SceneVS / ScenePS  — lit scene objects rendered into an offscreen texture
//   PostVS  / PostPS   — fullscreen quad with tone-map + warm tint + vignette

#pragma pack_matrix(row_major)

// ---------------------------------------------------------------------------
// Scene pass
// ---------------------------------------------------------------------------

// b0: per-frame CBV (uploaded via persistently mapped buffer)
cbuffer SceneFrameConstants : register(b0)
{
    float4x4 ViewProj;
    float3   LightDir;   float _p0;
    float3   CameraPos;  float _p1;
};

// b1: per-object root constants  (16 + 3 + 1 = 20 floats)
cbuffer SceneObjectConstants : register(b1)
{
    float4x4 WorldMatrix;
    float3   DiffuseColor;
    float    SpecularPower;
};

struct SceneVertexIn
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct SceneVertexOut
{
    float4 ClipPos  : SV_POSITION;
    float3 WorldPos : WORLDPOS;
    float2 UV       : TEXCOORD;
};

SceneVertexOut SceneVS(SceneVertexIn input)
{
    SceneVertexOut o;
    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    o.ClipPos  = mul(worldPos, ViewProj);
    o.WorldPos = worldPos.xyz;
    o.UV       = input.TexCoord;
    return o;
}

float4 ScenePS(SceneVertexOut input) : SV_TARGET
{
    // Reconstruct a hemisphere normal from UV coordinates.
    // UV [0,1] -> uv2 [-1,1]; then N = normalize(uv2.x, uv2.y, sqrt(1 - r2)).
    // Fragments outside the unit circle use a flat upward normal (floor objects).
    float2 uv2 = input.UV * 2.0f - 1.0f;
    float  r2  = dot(uv2, uv2);

    float3 normal;
    if (r2 > 1.0f)
    {
        // Outside the hemisphere dome — treat as flat upward face (floor)
        normal = float3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        normal = normalize(float3(uv2.x, uv2.y, sqrt(max(0.0f, 1.0f - r2))));
    }

    // Phong lighting
    float3 L = normalize(LightDir);
    float3 V = normalize(CameraPos - input.WorldPos);
    float3 H = normalize(L + V);

    float  NdotL    = max(dot(normal, L), 0.0f);
    float  NdotH    = max(dot(normal, H), 0.0f);

    float3 ambient  = float3(0.05f, 0.05f, 0.06f) * DiffuseColor;
    float3 diffuse  = NdotL * DiffuseColor;
    float3 specular = pow(NdotH, max(SpecularPower, 1.0f)) * float3(0.8f, 0.8f, 0.9f);

    float3 color = saturate(ambient + diffuse + specular);
    return float4(color, 1.0f);
}

// ---------------------------------------------------------------------------
// Post-process pass
// ---------------------------------------------------------------------------

Texture2D    SceneTexture  : register(t0);
SamplerState LinearSampler : register(s0);

// 4 root constants at b0: time, vignette strength, unused x2
cbuffer PostConstants : register(b0)
{
    float Time;
    float VignetteStrength;
    float _u0;
    float _u1;
};

struct PostVertexOut
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

// Fullscreen quad generated from SV_VertexID — no vertex buffer needed.
PostVertexOut PostVS(uint vid : SV_VertexID)
{
    // Two triangles covering NDC [-1,1]:
    //  vid: 0=TL, 1=TR, 2=BL, 3=TR, 4=BR, 5=BL
    float2 uv;
    uv.x = (vid == 1 || vid == 3 || vid == 4) ? 1.0f : 0.0f;
    uv.y = (vid == 2 || vid == 4 || vid == 5) ? 1.0f : 0.0f;

    PostVertexOut o;
    o.Position = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f);
    o.UV       = uv;
    return o;
}

float4 PostPS(PostVertexOut input) : SV_TARGET
{
    float3 color = SceneTexture.Sample(LinearSampler, input.UV).rgb;

    // Reinhard tone mapping
    color = color / (color + 1.0f);

    // Warm tint: slightly boost red, suppress blue
    color *= float3(1.05f, 1.0f, 0.92f);

    // Vignette: darken edges, strength driven by root constant
    float2 vigUV   = input.UV - 0.5f;
    float  vignette = 1.0f - dot(vigUV, vigUV) * (1.5f * VignetteStrength);
    color *= saturate(vignette);

    return float4(saturate(color), 1.0f);
}
