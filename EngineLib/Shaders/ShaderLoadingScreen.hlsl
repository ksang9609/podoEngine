Texture2D LoadingTexture : register(t0);
SamplerState LoadingSampler : register(s0);

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
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float3 color = LoadingTexture.Sample(LoadingSampler, input.uv).rgb;

    // DDS의 알파를 무시하고 완전 불투명으로 출력
    return float4(color, 1.0f);
}
