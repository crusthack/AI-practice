// Descriptor Heap.hlsl
//
// The CBV in the shader-visible descriptor heap carries scene-level constants that apply
// uniformly to all seven triangles drawn this frame (one CBV, many objects).
// Compare with 05 where per-object data was packed into root constants instead.

cbuffer SceneConstants : register(b0)
{
    float2 gOffset; // Lissajous position offset written each frame by the CPU
    float  gCosA;   // precomputed cos(scene rotation angle)
    float  gSinA;   // precomputed sin(scene rotation angle)
    float4 gTint;   // global colour multiplier
};

struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    float2 pos = input.position.xy;

    // Rotate the entire star around the origin using angles from the CBV.
    float2 rotated = float2(gCosA * pos.x - gSinA * pos.y,
                            gSinA * pos.x + gCosA * pos.y);

    PSInput output;
    output.position = float4(rotated + gOffset, input.position.z, 1.0f);
    output.color    = input.color * gTint;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
