#include "../CommonMath.hlsli"

RWTexture2D<float4> UAV : register(u0);

cbuffer BlurParams
{
    int kernelSize;
};

[numthreads(8, 8, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    const int blurSize = kernelSize;
    const int halfBlur = blurSize / 2;
    float4 result = 0.0;
    for (int x = -halfBlur; x < -halfBlur + blurSize; ++x)
    {
        for (int y = -halfBlur; y < -halfBlur + blurSize; ++y)
        {
            float2 offset = float2(float(x), float(y));
            result += UAV[threadID + offset];
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    UAV[threadID] = result / pow(blurSize, 2);
}