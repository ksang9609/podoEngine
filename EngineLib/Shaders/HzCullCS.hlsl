struct FWorldAABB
{
    float3 Min;
    float Pad1;
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
    float2 ScreenSize;
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

    float minClipW = 10000.0f;
    
    // Check Clipping AABB corners from Project and Near Plane
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        float4 clipPos = mul(float4(corners[i], 1.0f), ViewProjection);
        
        // Get NDC
        float3 ndc = clipPos.xyz / clipPos.w;
        minNDC = min(minNDC, ndc);
        maxNDC = max(maxNDC, ndc);
    }


    if (minClipW < 300.0f) // 프로젝트 월드 단위에 맞춰 200~400 조절
    {
        OutVisibility[index] = 1;
        return;
    }
    
    // Out of screen
     if(maxNDC.x < -1.0f || minNDC.x > 1.0f ||
        maxNDC.y < -1.0f || minNDC.y > 1.0f ||
        maxNDC.z < -1.0f || minNDC.z > 1.0f)
    {
        OutVisibility[index] = 0;
        return;
    }
    
    // NDC -> UV
    float2 minUV;
    minUV.x = saturate(minNDC.x * 0.5f + 0.5f);
    minUV.y = saturate(maxNDC.y * -0.5f + 0.5f);
    
    float2 maxUV;
    maxUV.x = saturate(maxNDC.x * 0.5f + 0.5f);
    maxUV.y = saturate(minNDC.y * -0.5f + 0.5f);

    float2 uvCenter = (minUV + maxUV) * 0.5f;
    float2 uvHalfExtent = (maxUV - minUV) * 0.5f * 0.8f;
    float2 sampleMinUV = clamp(uvCenter - uvHalfExtent, 0.001f, 0.999f);
    float2 sampleMaxUV = clamp(uvCenter + uvHalfExtent, 0.001f, 0.999f);

    
    // Get PixelSize and MipLevel    
    float2 pixelSize = (maxUV - minUV) * ScreenSize;
    if (max(pixelSize.x, pixelSize.y) > 60.0f)
    {
        OutVisibility[index] = 1;
        return;
    }
    
    uint mipLevel = (uint) clamp(floor(log2(max(pixelSize.x, pixelSize.y))) - 1.0f, 0.0f, 10.0f);
    //uint mipLevel = 0;
    
    // Get HiZDepth
    //float d0 = HiZTexture.SampleLevel(PointClampSampler, float2(minUV.x, minUV.y), mipLevel);
    //float d1 = HiZTexture.SampleLevel(PointClampSampler, float2(maxUV.x, minUV.y), mipLevel);
    //float d2 = HiZTexture.SampleLevel(PointClampSampler, float2(minUV.x, maxUV.y), mipLevel);
    //float d3 = HiZTexture.SampleLevel(PointClampSampler, float2(maxUV.x, maxUV.y), mipLevel);

    //float2 centerUV = (minUV + maxUV) * 0.5f;
    //float dCenter = HiZTexture.SampleLevel(PointClampSampler, centerUV, mipLevel);

    float d0 = HiZTexture.SampleLevel(PointClampSampler, float2(sampleMinUV.x, sampleMinUV.y), mipLevel);
    float d1 = HiZTexture.SampleLevel(PointClampSampler, float2(sampleMaxUV.x, sampleMinUV.y), mipLevel);
    float d2 = HiZTexture.SampleLevel(PointClampSampler, float2(sampleMinUV.x, sampleMaxUV.y), mipLevel);
    float d3 = HiZTexture.SampleLevel(PointClampSampler, float2(sampleMaxUV.x, sampleMaxUV.y), mipLevel);
    float dCenter = HiZTexture.SampleLevel(PointClampSampler, uvCenter, mipLevel);
    
    float maxHiZDepth = max(max(max(d0, d1), max(d2, d3)), dCenter);
    
    float minObjectDepth = minNDC.z;
    //float linearDist = max(minClipW, 1.0f);
    //float bias = clamp(0.015f / linearDist, 0.001f, 0.0025f);
    float bias = 0.003f;
    
    OutVisibility[index] = (minObjectDepth <= maxHiZDepth + bias) ? 1 : 0;    
}
