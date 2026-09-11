Texture2D TestTexture : register(t0);
SamplerState TestSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    // 첫 테스트는 행렬 변환 없이 화면에 직접 출력한다.
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;

    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return TestTexture.Sample(TestSampler, input.uv);
}
