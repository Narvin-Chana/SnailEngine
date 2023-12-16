#include "../CommonMath.hlsli"
#include "../GaussianKernel.hlsli"

Texture2D halfResDepth : register(t0);

RWTexture2D<float3> UAV : register(u0);

static const int halfSamples = 7;
static const float gaussFilterWeights[8] = { 0.14446445, 0.13543542, 0.11153505, 0.08055309, 0.05087564, 0.02798160, 0.01332457, 0.00545096 };
static const float blurDepthFalloff = 1000.0f;

// Function taken from Benjamin Glatzel's Volumetric Lighting talk : 
// https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen
// Slide 77
float3 GatherGauss(int2 blurDirection, int2 pos)
{
    float centerDepth = halfResDepth.Load(float3(pos, 0)).r;
    
    float3 accumResult = 0;
    float accumWeights = 0;
    
    for (int r = -halfSamples; r <= halfSamples; ++r)
    {
        int2 offset = r * blurDirection;
        float3 kernelSample = UAV[pos + offset];
        float kernelDepth = halfResDepth.Load(float3(pos + offset, 0)).r;

        float depthDiff = abs(kernelDepth - centerDepth);
        float r2 = blurDepthFalloff * depthDiff;
        float g = exp(-r2 * r2);
        float weight = g * gaussFilterWeights[abs(r)];

        accumResult += weight * kernelSample;
        accumWeights += weight;
    }
    
    return accumResult / accumWeights;
}

[numthreads(8, 8, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    // Horizontal Pass
    UAV[threadID] = GatherGauss(int2(1, 0), threadID);

    GroupMemoryBarrierWithGroupSync();
    
    // Vertical Pass
    UAV[threadID] = GatherGauss(int2(0, 1), threadID);
}