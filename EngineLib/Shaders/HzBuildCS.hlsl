Texture2D<float> InSourceTexture : register(t0); // read
RWTexture2D<float> OutDestMip : register(u0); // write

SamplerState PointClampSampler : register(s0);

cbuffer HzBuildConstants : register(b0)
{
    uint2 inputOffset;
    uint2 inputSize;
    uint2 outputSize;
    uint2 pad;
};

[numthreads(16, 16, 1)]
void mainCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint2 texel = DispatchThreadID.xy;
    
    if (texel.x >= outputSize.x || texel.y >= outputSize.y)
    {
        return;
    }
    
    uint2 srcCoord = inputOffset + (texel * inputSize) / outputSize;

    // 2x2 Down Sampling
    float d1 = InSourceTexture.Load(int3(srcCoord + int2(0, 0), 0));
    float d2 = InSourceTexture.Load(int3(srcCoord + int2(1, 0), 0));
    float d3 = InSourceTexture.Load(int3(srcCoord + int2(0, 1), 0));
    float d4 = InSourceTexture.Load(int3(srcCoord + int2(1, 1), 0));

    float maxDepth = max(max(d1, d2), max(d3, d4));
    
    OutDestMip[texel] = maxDepth;
}
