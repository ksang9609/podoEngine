struct FWorldAABB
{
    float3 Min;
    uint InternalID;
    float3 Max;
    float Pad2;
};

Texture2D<float> HiZTexture : register(t0);
StructuredBuffer<FWorldAABB> InAABBs : register(t1);

RWStructuredBuffer<uint> OutVisibility : register(u0);

SamplerState PointClampSampler : register(s0);

cbuffer CullConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float2 HZBSize;
    uint NumObjects;
    float NearPlane;
};

[numthreads(64, 1, 1)]
void mainCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint index = DispatchThreadID.x;
    if(index >= NumObjects)
    {
        return;
    }

    FWorldAABB aabb = InAABBs[index];

    // World AABB corners
    float3 corners[8];
    corners[0] = float3(aabb.Min.x, aabb.Min.y, aabb.Min.z);
    corners[1] = float3(aabb.Max.x, aabb.Min.y, aabb.Min.z);
    corners[2] = float3(aabb.Min.x, aabb.Max.y, aabb.Min.z);
    corners[3] = float3(aabb.Max.x, aabb.Max.y, aabb.Min.z);
    corners[4] = float3(aabb.Min.x, aabb.Min.y, aabb.Max.z);
    corners[5] = float3(aabb.Max.x, aabb.Min.y, aabb.Max.z);
    corners[6] = float3(aabb.Min.x, aabb.Max.y, aabb.Max.z);
    corners[7] = float3(aabb.Max.x, aabb.Max.y, aabb.Max.z);

    float3 minNDC = float3(100.0f, 100.0f, 100.0f);
    float3 maxNDC = float3(-100.0f, -100.0f, -100.0f);
    
    bool bIntersectNearPlane = false;
    bool bAllBehind = true;
    
    // Check Clipping AABB corners from Project and Near Plane
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        float4 clipPos = mul(float4(corners[i], 1.0f), ViewProjection);
        
        if (clipPos.w <= 0.0f)
        {
            bIntersectNearPlane = true;
        }
        else
        {
            bAllBehind = false;
            // Get NDC
            float3 ndc = clipPos.xyz / clipPos.w;
            minNDC = min(minNDC, ndc);
            maxNDC = max(maxNDC, ndc);
        }
    }   
    
    if (bAllBehind)
    {
        OutVisibility[aabb.InternalID] = 0;
        return;
    }
    if (bIntersectNearPlane)
    {
        OutVisibility[aabb.InternalID] = 1;
        return;
    }
    
    // NDC -> UV
    float2 minUV;
    minUV.x = saturate(minNDC.x * 0.5f + 0.5f);
    minUV.y = saturate(maxNDC.y * -0.5f + 0.5f);
    
    float2 maxUV;
    maxUV.x = saturate(maxNDC.x * 0.5f + 0.5f);
    maxUV.y = saturate(minNDC.y * -0.5f + 0.5f);

    // Get PixelSize and MipLevel    
    float2 pixelSize = (maxUV - minUV) * HZBSize;
    uint mipLevel = (uint)ceil(log2(max(max(pixelSize.x, pixelSize.y), 1.0f)));
    mipLevel = min(mipLevel, 10);

    uint2 mipSize = max(uint2(1, 1), (uint2) HZBSize >> mipLevel);
    uint2 maxCoord = mipSize - uint2(1, 1);

    #define SAMPLE_HIZ(uv) HiZTexture.Load(int3(min((uint2)(saturate(uv) * mipSize), maxCoord), (int)mipLevel))

    float2 uvCenter = (minUV + maxUV) * 0.5f;
    float hiZDepth = SAMPLE_HIZ(uvCenter);

    hiZDepth = max(hiZDepth, SAMPLE_HIZ(minUV));
    hiZDepth = max(hiZDepth, SAMPLE_HIZ(maxUV));
    hiZDepth = max(hiZDepth, SAMPLE_HIZ(float2(minUV.x, maxUV.y)));
    hiZDepth = max(hiZDepth, SAMPLE_HIZ(float2(maxUV.x, minUV.y)));
    #undef SAMPLE_HIZ
    
    // Get HiZDepth
    //float d0 = HiZTexture.SampleLevel(PointClampSampler, float2(minUV.x, minUV.y), mipLevel);
    //float d1 = HiZTexture.SampleLevel(PointClampSampler, float2(maxUV.x, minUV.y), mipLevel);
    //float d2 = HiZTexture.SampleLevel(PointClampSampler, float2(minUV.x, maxUV.y), mipLevel);
    //float d3 = HiZTexture.SampleLevel(PointClampSampler, float2(maxUV.x, maxUV.y), mipLevel);

    //float maxHiZDepth = max(max(d0, d1), max(d2, d3));
    
    float minObjectDepth = minNDC.z;
    float bias = 0.00001f;
        
    OutVisibility[aabb.InternalID] = (minObjectDepth <= hiZDepth + bias) ? 1 : 0;
}
