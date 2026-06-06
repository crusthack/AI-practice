// Stage 22: Lighting And Materials
// Demonstrates per-pixel Phong lighting. Normals are derived from UV coordinates to
// simulate a sphere surface, so the lighting math is visible without 3D mesh geometry.

cbuffer PerFrame : register(b0)
{
    float3 LightDir;       // normalised direction toward the light source
    float  _p0;
    float3 AmbientColor;
    float  _p1;
    float  TimeSeconds;
    float3 _p2;
};

cbuffer PerObject : register(b1)
{
    float3 DiffuseColor;
    float  SpecularPower;
    float3 SpecularColor;
    float  Roughness;
    float  PosX;
    float  PosY;
    float  Scale;
    float  AspectH;    // viewport width / height (corrects circle aspect ratio)
};

struct VertexIn
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut o;
    // Scale the unit quad and translate to object centre in NDC.
    float2 p = input.Position * Scale;
    p.x += PosX;
    p.y += PosY;
    o.Position = float4(p, 0.0f, 1.0f);
    o.UV = input.TexCoord;
    return o;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    // Remap UV to [-1,1] and correct for aspect ratio to make a true circle mask.
    float2 uv = input.UV * 2.0f - 1.0f;
    uv.x *= AspectH;  // squeeze x to account for non-square viewport when Scale is equal
    // Discard fragments outside the unit circle.
    float r2 = dot(uv, uv);
    if (r2 > 1.0f) discard;

    // Reconstruct a hemisphere normal from the 2D UV position.
    float3 normal = float3(uv, sqrt(max(0.0f, 1.0f - r2)));
    normal = normalize(normal);

    // Phong: ambient + diffuse + specular
    float3 L = normalize(LightDir);
    float3 V = float3(0.0f, 0.0f, 1.0f); // viewer always along +Z in this 2D demo
    float3 H = normalize(L + V);

    float NdotL = max(dot(normal, L), 0.0f);
    float NdotH = max(dot(normal, H), 0.0f);

    float3 ambient  = AmbientColor * DiffuseColor;
    float3 diffuse  = NdotL * DiffuseColor;
    float3 specular = pow(NdotH, max(SpecularPower, 1.0f)) * SpecularColor * (1.0f - Roughness);

    float3 color = saturate(ambient + diffuse + specular);

    // Soft edge for a smooth sphere silhouette
    float edgeFade = smoothstep(1.0f, 0.92f, sqrt(r2));
    return float4(color * edgeFade, edgeFade);
}
