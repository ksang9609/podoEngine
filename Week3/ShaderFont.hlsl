cbuffer constants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 Tint;
}

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(mul(float4(input.position, 1.0f), World), ViewProjection);
    output.texCoord = input.texCoord;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    const float4 atlasColor = FontAtlas.Sample(FontSampler, input.texCoord);
    // Alpha 값이 거의 0인 픽셀은 렌더링하지 않는다
    clip(atlasColor.a - 0.01f);
    return float4(Tint.rgb * atlasColor.rgb, Tint.a * atlasColor.a);
}
