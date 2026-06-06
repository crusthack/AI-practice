cbuffer ClearColorCB : register(b0)
{
    float4 ClearColor;
}

struct PSInput
{
    float4 Pos : SV_POSITION;
};

PSInput VS(uint vertexId : SV_VertexID)
{
    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f,  3.0f),
        float2( 3.0f, -1.0f)
    };

    PSInput output;
    output.Pos = float4(positions[vertexId], 0.0f, 1.0f);
    return output;
}

float4 PS(PSInput input) : SV_Target
{
    return ClearColor;
}
