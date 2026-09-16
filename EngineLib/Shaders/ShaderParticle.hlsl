Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

cbuffer gridConstants : register(b0)
{
    float numRows;
    float numCols;
    int currentFrame;
    int nextFrame;   

    float frameRatio;
}
