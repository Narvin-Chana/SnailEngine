#ifndef __SHADOWS_HLSL__
#define __SHADOWS_HLSL__

#ifndef __LIGHTS_DEF_HLSL__
#include "LightsDef.hlsli"
#endif

struct ShadowComputeData 
{
    float3 worldPosition;
    float3 normal;
    float depth;
};

// Use "PCF" define to enable soft shadows

// This needs to match CASCADE_COUNT in DirectionalShadows.h
static const uint MAX_DIR_LIGHTS_CASCADE = 6;

struct CascadeShadow {
    matrix transformMatrixes;
    float2 cascadeBoundaries;
    float2 _pad; // Force padding on 16 byte frontier for array packing
};

struct CascadeShadows {
    CascadeShadow cascades[MAX_DIR_LIGHTS_CASCADE];
};

matrix GetShadowMatrix(CascadeShadows shadows, uint cascadeId)
{
    return shadows.cascades[cascadeId].transformMatrixes;
}

float GetCascadeLowerBound(CascadeShadow shadow)
{
    return shadow.cascadeBoundaries.x;
}

float GetCascadeUpperBound(CascadeShadow shadow)
{
    return shadow.cascadeBoundaries.y;
}

int GetCascadeIndex(CascadeShadows shadows, float distanceFromCamera, int startOffset = 0)
{
    if (startOffset < 0)
        return -1;
    
    for (uint i = startOffset; i < MAX_DIR_LIGHTS_CASCADE; ++i) 
    {
        if (distanceFromCamera > shadows.cascades[i].cascadeBoundaries.x && 
            distanceFromCamera < shadows.cascades[i].cascadeBoundaries.y)
        {
            return i;
        }
    }
    
    return -1;
}

SamplerComparisonState ShadowSamplerComp
{
    ComparisonFunc = GREATER;
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
};

float CalculateShadowFactor(int lightIndex, float4 posFromLight, float bias, Texture2DArray shadowMap, SamplerState shadowSampler, int cascadeId, int kernelSize)
{
    float3 projCoords = posFromLight.xyz / posFromLight.w;
    const float currentDepth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.x < 0 || projCoords.y < 0 || projCoords.x > 1 || projCoords.y > 1)
        return 0;
    
    float2 sampleCoord = float2(projCoords.x, 1 - projCoords.y);

    int layer = lightIndex * MAX_DIR_LIGHTS_CASCADE + cascadeId;

#ifdef PCF
    uint w, h, n, count;
    shadowMap.GetDimensions(0, w, h, count, n);

    int total = 0;
    
    // Change this for smoother shadows
    int halfSizeKernel = kernelSize / 2;
    
    for (int x = -halfSizeKernel; x < -halfSizeKernel + kernelSize; x++)
    {
        for (int y = -halfSizeKernel; y < -halfSizeKernel + kernelSize; y++)
        {
            total += shadowMap.SampleCmpLevelZero(ShadowSamplerComp, float3(sampleCoord, layer), currentDepth - bias, int2(x, y));
        }
    }
    
    return total / pow(kernelSize, 2.0f);
#endif
    
    return shadowMap.SampleCmpLevelZero(ShadowSamplerComp, float3(sampleCoord, layer), currentDepth - bias);
}

float ComputeShadowForCascade(DirectionalLight light, int lightIndex, ShadowComputeData computeData, Texture2DArray shadowMap, SamplerState shadowSampler, CascadeShadows cascades, int cascadeId, int kernelSize)
{
    const matrix cascadeMatrix = GetShadowMatrix(cascades, cascadeId);
    const float4 posFromLight = mul(float4(computeData.worldPosition + computeData.normal * ((cascadeId + 1) * 0.2), 1), cascadeMatrix);
    return CalculateShadowFactor(lightIndex, posFromLight, 0, shadowMap, shadowSampler, cascadeId, kernelSize);
}

float3 CalculateShadowFactorWithBlend(DirectionalLight dirLight, uint lightIndex, ShadowComputeData computeData, Texture2DArray shadowMap, SamplerState shadowSampler, CascadeShadows cascadeShadow, int kernelSize)
{
    const float distanceFromCam = computeData.depth;
            
    const int cascadeId = GetCascadeIndex(cascadeShadow, distanceFromCam);
    if (cascadeId == -1)
        return 1;
            
    float shadow = ComputeShadowForCascade(dirLight, lightIndex, computeData, shadowMap, shadowSampler, cascadeShadow, cascadeId, kernelSize);
            
    float3 _shadow = shadow;
#ifdef DEBUG_SHADOWS
    // Shows different colors for cascades
    if (cascadeId == 0)
        _shadow *= float3(1, 0, 0);
    if (cascadeId == 1)
        _shadow *= float3(1, 1, 0);
    if (cascadeId == 2)
        _shadow *= float3(0, 1, 0);
    if (cascadeId == 3)
        _shadow *= float3(0, 1, 1);
    if (cascadeId == 4)
        _shadow *= float3(0, 0, 1);
    if (cascadeId == 5)
        _shadow *= float3(1, 0, 1);
#endif

    const int cascadeId2 = GetCascadeIndex(cascadeShadow, distanceFromCam, cascadeId + 1);
            
    // If the position is also in the next cascade, calculate that shadow value and blend
    if (cascadeId2 != -1)
    {
        // Fetch the lower bound of the next cascade and the upper bound of the previous
        float upperBound = GetCascadeUpperBound(cascadeShadow.cascades[cascadeId]);
        float lowerBound = GetCascadeLowerBound(cascadeShadow.cascades[cascadeId2]);
            
        float blendShadow = ComputeShadowForCascade(dirLight, lightIndex, computeData, shadowMap, shadowSampler, cascadeShadow, cascadeId2, kernelSize);
                
        float3 _blendShadow = blendShadow;
#ifdef DEBUG_SHADOWS
        // Shows different colors for cascades
        if (cascadeId2 == 0)
            _blendShadow *= float3(1, 0, 0);
        if (cascadeId2 == 1)
            _blendShadow *= float3(1, 1, 0);
        if (cascadeId2 == 2)
            _blendShadow *= float3(0, 1, 0);
        if (cascadeId2 == 3)
            _blendShadow *= float3(0, 1, 1);
        if (cascadeId2 == 4)
            _blendShadow *= float3(0, 0, 1);
        if (cascadeId == 5)
            _blendShadow *= float3(1, 0, 1);
#endif
        // This calculates the blend ratio
        _shadow = lerp(_shadow, _blendShadow, (distanceFromCam - lowerBound) / (upperBound - lowerBound));
    }
    
    return _shadow;
}

#endif