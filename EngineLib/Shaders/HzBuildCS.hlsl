Texture2D<float> InSourceTexture : register(t0); // read
RWTexture2D<float> OutDestMip : register(u0); // write

SamplerState PointClampSampler : register(s0);

cbuffer HzBuildConstants : register(b0)
{
    uint2 DstDimensions; // Width, Height
    float2 InvDstDimensions; // 1 / DstDimensions
    float2 ViewportUVOffset;
    float2 ViewportUVScale;
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
        // 1. [0, 1] HZB UV 좌표 도출
        float2 localUV = (float2(DispatchThreadID.xy) + 0.5f) * InvDstDimensions;
        // 2. [핵심] 실제 깊이 버퍼 상의 뷰포트 위치로 UV 변환!
        float2 actualUV = ViewportUVOffset + localUV * ViewportUVScale;
        // 3. 샘플링 (GatherRed 대신 안전한 4점 샘플링 사용)
        float2 texelSize = float2(1.0f / 1920.0f, 1.0f / 1080.0f); // 미세 오프셋
        float d0 = InSourceTexture.SampleLevel(PointClampSampler, actualUV + float2(-0.5f, -0.5f) * texelSize, 0);
        float d1 = InSourceTexture.SampleLevel(PointClampSampler, actualUV + float2(0.5f, -0.5f) * texelSize, 0);
        float d2 = InSourceTexture.SampleLevel(PointClampSampler, actualUV + float2(-0.5f, 0.5f) * texelSize, 0);
        float d3 = InSourceTexture.SampleLevel(PointClampSampler, actualUV + float2(0.5f, 0.5f) * texelSize, 0);
        maxDepth = max(max(d0, d1), max(d2, d3));        
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
