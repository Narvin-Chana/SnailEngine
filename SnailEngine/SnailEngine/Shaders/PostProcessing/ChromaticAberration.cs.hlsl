#include "../CommonMath.hlsli"

RWTexture2D<float4> UAV : register(u0);

cbuffer ChromaticAberrationParameters
{
    float3 rgbOffset;
    float intensity;
};

[numthreads(16, 16, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    float4 color = UAV[threadID];
    color.r = UAV[threadID + rgbOffset.rr * intensity].r;
    color.g = UAV[threadID + rgbOffset.gg * intensity].g;
    color.b = UAV[threadID + rgbOffset.bb * intensity].b;
    GroupMemoryBarrierWithGroupSync();
    UAV[threadID] = color;
}