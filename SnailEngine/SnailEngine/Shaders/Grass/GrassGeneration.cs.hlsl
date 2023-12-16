#include "GrassDef.hlsli"
#include "../CommonMath.hlsli"
#include "../Noise.hlsl"

cbuffer GrassParams
{
    matrix GrassDensityScaleMatrix;
    float3 WorldPosition;
    uint2 GroupCount;
};

static const int ThreadCount = 8;

RWStructuredBuffer<GrassInstanceData> instanceDataBuffer;

#ifdef SAMPLE_GRASS
Texture2D PatchSample : register(t0);
SamplerState PatchSampleSampler : register(s0);
#endif

[numthreads(ThreadCount, ThreadCount, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint2 threadID : SV_GroupThreadID, uint2 groupID : SV_GroupID)
{
    // Add grass blade vertex to vertex buffer
    GrassInstanceData blade = (GrassInstanceData)0;
    
    float3 position = mul(float4(dispatchThreadID.x, 0, dispatchThreadID.y, 1), GrassDensityScaleMatrix).xyz;

    float4 hashVal = Hash42(float2(position.x, position.z));
    
#ifdef SAMPLE_GRASS
    float isGrassAllowed = PatchSample.SampleLevel(PatchSampleSampler, float2(1 - position.x / (GroupCount.x * ThreadCount * GrassDensityScaleMatrix._11), position.z / (GroupCount.y * ThreadCount * GrassDensityScaleMatrix._33)), 0).x == 0;
#else
    float isGrassAllowed = 1;
#endif    
    blade.randomAngle = hashVal.x * 2.0 * MATHS_PI;
    blade.randomShade = remap(hashVal.y, -1.0, 1.0, 0.5, 1.0);
    // INFO: These TODOs won't be done before the end of the project. They are left here to inform the future reader of potential improvements.
    // TODO: add lods (or just don't draw grass after a certain distance)
    // TODO: add heightmapSampleHeight somehow to have the grass follow the terrain's height
    blade.randomHeight = remap(hashVal.z, 0.0, 1.0, 0.85, 1.10) /** mix(1.0, 0.0, lodFadeIn)*/ * isGrassAllowed /** heightmapSampleHeight*/;
    blade.randomLean = remap(hashVal.w, 0.0, 1.0, 0.1, 0.4);
    
    float2 hashGrassColour = Hash22(float2(position.x, position.z));

    float3 b1 = float3(0.02, 0.9, 0.01);
    float3 b2 = float3(0.025, 0.6, 0.01);
    float3 t1 = float3(0.65, 0.8, 0.25);
    float3 t2 = float3(0.8, 1, 0);
    
    blade.baseColor = lerp(b1, b2, hashGrassColour.x);
    blade.tipColor = lerp(t1, t2, hashGrassColour.y);

    float2 offset = float2(hashVal.x + hashVal.y, hashVal.z + hashVal.w);
    
    blade.worldPosition = float3(position.x + offset.x, position.y, position.z + offset.y);
    
    // Flatten coordinates to write to instanceDataBuffer
    instanceDataBuffer[dispatchThreadID.x + dispatchThreadID.y * (GroupCount.x * ThreadCount)] = blade;
}

