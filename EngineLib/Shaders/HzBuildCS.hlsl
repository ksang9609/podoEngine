Texture2D<float> InSourceTexture : register(t0); // read
RWTexture2D<float> OutDestMip : register(u0); // write

SamplerState PointClampSampler : register(s0);

cbuffer HzBuildConstants : register(b0)
{
    uint2 DstDimensions; // Width, Height
    float2 InvDstDimensions; // 1 / DstDimensions
    float2 SrcTexelSize;
    uint IsPass0;
};

[numthreads(16, 16, 1)]
void mainCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // not target mipmap size
    if (DispatchThreadID.x >= DstDimensions.x || DispatchThreadID.y >= DstDimensions.y)
    {
        return;
    }

    float maxDepth = 0.0f;

    if(IsPass0)
    {
        float2 uv = (float2(DispatchThreadID.xy) + 0.5f) * InvDstDimensions;
        float4 depths = InSourceTexture.GatherRed(PointClampSampler, uv);
        maxDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));
    }
    else
    {
        uint2 srcCoord = DispatchThreadID.xy * 2;

        // 2x2 Down Sampling
        float d1 = InSourceTexture.Load(int3(srcCoord + int2(0, 0), 0));
        float d2 = InSourceTexture.Load(int3(srcCoord + int2(1, 0), 0));
        float d3 = InSourceTexture.Load(int3(srcCoord + int2(0, 1), 0));
        float d4 = InSourceTexture.Load(int3(srcCoord + int2(1, 1), 0));

        maxDepth = max(max(d1, d2), max(d3, d4));
    }  
    
    OutDestMip[DispatchThreadID.xy] = maxDepth;
}
