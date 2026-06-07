struct ReadbackColor
{
    float R;
    float G;
    float B;
    float A;
};

RWStructuredBuffer<ReadbackColor> OutputColor : register(u0);

cbuffer ReadbackConstants : register(b0)
{
    float TimeSeconds;
};

[numthreads(1, 1, 1)]
void ComputeMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    ReadbackColor color;
    color.R = 0.24f + 0.22f * sin(TimeSeconds * 1.3f);
    color.G = 0.38f + 0.24f * sin(TimeSeconds * 1.9f + 1.4f);
    color.B = 0.18f + 0.18f * sin(TimeSeconds * 1.1f + 2.2f);
    color.A = 1.0f;
    OutputColor[0] = color;
}
