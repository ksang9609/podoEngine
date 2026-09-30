Texture2D<float> InSourceTexture : register(t0);
RWStructuredBuffer<uint> OutDesMip : register(u0);

SamplerState PointClampSampler : register(s0);

cbuffer HzBuildConstants : register(b0)
{
    
}

[numthreads(16, 16, 1)]
void mainCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    
}
