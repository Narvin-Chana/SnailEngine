#include "../CommonMath.hlsli"

RWTexture2D<float4> UAV : register(u0);

cbuffer SSAOParams{
    matrix invProjMat;
    matrix projMat;
    matrix invViewMat;
    float4 samples[64];
    int2 size;
    int kernelSize;
    float radius;
    float bias;
    float power;
};

Texture2DArray GBuffer;
Texture2D Depth;
Texture2D Noise;
SamplerState NoiseSampler;

float3 GetPos(float2 clipPoint)
{
    float2 pos = clipPoint * size;
    clipPoint.y = 1 - clipPoint.y;
    return DepthToWorld(Depth, invProjMat, pos, clipPoint);
}

[numthreads(8, 8, 1)]
void main(uint2 threadID : SV_DispatchThreadID)
{
    float2 uv = float2(threadID) / float2(size);
    float3 viewPos = GetPos(uv);
    
    float unlit = 1 - GBuffer.Load(int4(threadID, 5, 0)).x;
 
    float4 normWorldSpace = GBuffer.Load(int4(threadID, 1, 0)) * 2 - 1;
    float3 normal = normalize(mul(normWorldSpace, invViewMat).xyz);
    
    float2 noiseScale = size / 4.0f;
    float3 randomVec = normalize(Noise.SampleLevel(NoiseSampler, uv * noiseScale, 0).xyz);
    
    float3 tangent = normalize(randomVec - normal * dot(normal, randomVec));
    float3 bitangent = normalize(cross(tangent, normal));
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        float3 samplePos = mul(samples[i].xyz, TBN);
        samplePos = viewPos + samplePos * radius;

        // world or view space offset
        float4 offset = mul(float4(samplePos, 1.0), projMat);
        // clip space
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        offset.y = 1 - offset.y;
        
        // Get z from surface and sample point
        float sampleDepth = samplePos.z;
        float pixelDepth = GetPos(offset.xy).z;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pixelDepth - sampleDepth));
        occlusion += (sampleDepth - bias > pixelDepth ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = pow(1 - (occlusion / kernelSize), power);
    
    UAV[threadID] = float4(occlusion.xxx, 1) * unlit;
}