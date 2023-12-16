#include "../LightsDef.hlsli"
#include "../Shadows.hlsl"

Texture2DArray shadowMap : register(t0);
SamplerState shadowMapSampler : register(s0);

Texture2D DepthTexture : register(t1);

// Directional light scene data
// Struct of arrays is probably better cache-wise than an array of structs 
// Source: Slide 46 in Ubisoft GDC Talk: https://ubm-twvideo01.s3.amazonaws.com/o1/vault/gdc2015/presentations/Vaisse_Alexis_Ubisoft%20Cloth%20Simulation.pdf
// Also probably a better idea to pass all data in one cbuffer but requires a few changes elsewhere in the code
cbuffer AccumulationData : register(b0)
{
    matrix invProjectionMatrix;
    matrix invViewMatrix;
    uint nbDirectional;
    int4 randomRayIndices[16];
};

cbuffer DirectionalLights : register(b1)
{
    DirectionalLight dirLights[MAX_DIR_LIGHTS];
}

cbuffer CascadeShadows : register(b2)
{
    CascadeShadows shadows[MAX_DIR_LIGHTS];
}

RWTexture2D<float3> accumulationBuffer : register(u0);

static const uint StepCount = 16;
static const uint InterleavedGridSize = 8;
static const uint InterleavedGridSizeSqr = pow(InterleavedGridSize, 2);
static const float InterleavedGridSizeSqrRcp = 1.0f / InterleavedGridSizeSqr;

// UNUSED
static const float TAU = 0.0001;
static const float PHI = 10000000.0;
static const float PI_RCP = 0.31830988618;

float3 ComputeVolumetricLighting(DirectionalLight dirLight, CascadeShadows dirLightMatrix, int currentLightIndex, float3 posViewSpace, int interleavedOffset)
{
    float depth = posViewSpace.z;
    float3 viewSpacePosition = posViewSpace.xyz;
    float3 V = normalize(-viewSpacePosition);
    float3 L = dirLight.Direction;
    
    float stepSize = depth / StepCount;

    // Get V * stepSize vector in world space
    float4 rayMarchVectorWS = mul(float4(V * stepSize, 0), invViewMatrix);

    // Offset with interleaving to provide for less noticeable noise
    viewSpacePosition += V * stepSize * (1 + interleavedOffset * InterleavedGridSizeSqrRcp);
    
    ShadowComputeData shadowData;
    float4 worldPos = mul(float4(viewSpacePosition, 1), invViewMatrix);
    shadowData.worldPosition = worldPos.xyz;
    shadowData.normal = 0;
    
    float executedSamples = 0;
    float3 accumulation = 0;
    
    [loop]
    for (uint j = 0; j < StepCount; ++j)
    {
        float viewDepth = max(viewSpacePosition.z, 0.500001f); // clamp to near plane if closer
        if (viewDepth > depth)
            break;
        
        shadowData.depth = viewDepth;
        
        // Don't use PCF (we pass a kernel size of 1 as last param), otherwise performance TANKS
        float3 shadow = CalculateShadowFactorWithBlend(dirLight, currentLightIndex, shadowData, shadowMap, shadowMapSampler, dirLightMatrix, 1);
        
        // Could use radiative transport equation as defined in https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen
        // But it currently doesn't work with our implementation and we don't have enough time to fix this.
        //float d = length(viewSpacePosition);
        //float dRcp = rcp(d);
        //const float scatteringProbability = 0.25;
        //float3 intensity = TAU * (shadow * (PHI * scatteringProbability * PI_RCP) * dRcp * dRcp) * exp(-d * TAU) * exp(-(j * stepSize) * TAU) * stepSize;
        //accumulation += intensity;
        
        // This gives a realistic enough result
        accumulation += shadow;
        
        // Make worldPosition march parallel to view space ray to avoid a matrix mul each step
        viewSpacePosition += V * stepSize;
        shadowData.worldPosition += rayMarchVectorWS.xyz;
        executedSamples++;
    }
    accumulation /= executedSamples;
    
    return accumulation;
}

[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID, uint2 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    // Rebuild viewSpacePosition from depth
    float2 resolution;
    DepthTexture.GetDimensions(resolution.x, resolution.y);
    
    // Get actual pixel position to use from interleaved random array
    // We pack them into int4 for better memory transfer to GPU
    int interleavedOffset = ((int[4]) randomRayIndices[groupIndex / 4])[groupIndex % 4];

    int2 pos = dispatchThreadID * 2;
    
    // Get depth at this pixel
    float depth = DepthTexture.Load(float3(pos, 0)).r;
    float2 depthCoord = pos / resolution;
        
    // Obtain position of object at depth
    float3 screenPosition = float3((depthCoord.x * 2 - 1), ((1 - depthCoord.y) * 2 - 1), depth);
    float4 viewPosition = mul(float4(screenPosition, 1), invProjectionMatrix);
    viewPosition /= viewPosition.w;
    
    float3 accumulation = 0;
    
    uint i;
    for (i = 0; i < nbDirectional; ++i)
    {
        // Interleaved sampling attenuation calculation (with 16 samples)
        float3 attenuation = ComputeVolumetricLighting(dirLights[i], shadows[i], i, viewPosition.xyz, interleavedOffset);
        accumulation += attenuation * dirLights[i].Color;
    }

    accumulationBuffer[dispatchThreadID] = accumulation;
}
