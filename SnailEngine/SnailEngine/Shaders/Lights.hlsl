#ifndef __LIGHTS_HLSL__
#define __LIGHTS_HLSL__

#include "LightsDef.hlsli"
#include "Shadows.hlsl"

float3 DoDiffuse(const float3 color, const float3 L, float3 N, float3 V)
{
    const float NdotL = max(0, dot(N, L));
    return color * NdotL;
}

float DoAttenuation(float3 coefs, const float d) { return 1.0f / (coefs.x + coefs.y * d + coefs.z * pow(d, 2)); }

float3 ComputeBlinnPhong(float3 color, float3 V, float3 L, float3 N, LightComputeData computeData)
{
    // For normal maps
    const float3 H = normalize(L + V);
    const float NdotH = max(0, dot(N, H));

    return color * pow(NdotH, computeData.specularExp) * max(0, dot(L, N));
}

LightingResult ComputeSpotLighting(const SpotLight light, const LightComputeData input)
{
    const float3 cameraDirection = normalize(input.cameraPosition - input.worldPosition);
    const float3 lightDirection = normalize(light.Position - input.worldPosition);
    const float cutOff = cos(radians(light.InnerConeAngle)); // Take this as param
    const float outerCutOff = cos(radians(light.OuterConeAngle)); // Take this as param
    const float theta = dot(lightDirection, normalize(-light.Direction));
    const float epsilon = cutOff - outerCutOff;
    const float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    LightingResult result;

    const float attenuation = DoAttenuation(light.Coefs, distance(light.Position, input.worldPosition));
    result.diffuse = DoDiffuse(light.Color, -light.Direction, input.normal, cameraDirection) * attenuation * intensity;
    result.specular = ComputeBlinnPhong(
        light.Color,
        cameraDirection,
        -light.Direction,
        input.normal,
        input
    ) * attenuation * intensity;
    
    return result;
}

LightingResult ComputeDirectionalLighting(const DirectionalLight light, const LightComputeData input)
{
    const float3 cameraDirection = normalize(input.cameraPosition - input.worldPosition);

    LightingResult result;
    result.diffuse = DoDiffuse(light.Color, -normalize(light.Direction), input.normal, cameraDirection);
    result.specular = ComputeBlinnPhong(light.Color, cameraDirection, -normalize(light.Direction), input.normal, input);

    return result;
}

LightingResult ComputePointLighting(const PointLight light, const LightComputeData input)
{
    LightingResult result;

    const float3 cameraDirection = normalize(input.cameraPosition - input.worldPosition);
    const float3 lightDirection = normalize(light.Position - input.worldPosition);
    const float lightDist = distance(light.Position, input.worldPosition);
    const float attenuation = DoAttenuation(light.Coefs, lightDist);

    result.diffuse = DoDiffuse(light.Color, lightDirection, input.normal, cameraDirection) * attenuation;
    result.specular = ComputeBlinnPhong(
        light.Color,
        cameraDirection,
        lightDirection,
        input.normal,
        input
    ) * attenuation;

    return result;
}

static const float THIN_TRANSLUCENCY_FACTOR = 0.4;

LightingResult ComputeAllLighting(
    LightComputeData computeData,
    DirectionalLight dirLights[MAX_DIR_LIGHTS],
    SpotLight spotLights[MAX_SPOT_LIGHTS],
    PointLight pointLights[MAX_POINT_LIGHTS],
    const Texture2DArray shadowMap,
    const SamplerState shadowSampler,
    const CascadeShadows cascadeShadow[MAX_DIR_LIGHTS])
{
    LightingResult totalResult = (LightingResult)0;

    uint i;
    
    // Directional lights and Cascading Shadows
    for (i = 0; i < computeData.nbDirectional; ++i)
    {
        if (!dirLights[i].isActive)
            continue;
        
        LightingResult thinLighting = (LightingResult) 0;
        
        if (computeData.isTranslucent)
        {
            LightComputeData data = computeData;
            data.normal = -data.normal;
            thinLighting = ComputeDirectionalLighting(dirLights[i], data);
        }
        
        LightingResult lightingResult = ComputeDirectionalLighting(dirLights[i], computeData);
        
        if (dirLights[i].castsShadows)
        {
            ShadowComputeData shadowData;
            shadowData.worldPosition = computeData.worldPosition;
            shadowData.normal = computeData.normal;
            shadowData.depth = computeData.posViewSpace.z;
            float3 surfaceShadow = CalculateShadowFactorWithBlend(dirLights[i], i, shadowData, shadowMap, shadowSampler, cascadeShadow[i], PCF_KERNEL_SIZE);
            
            float3 surfaceShadow2 = 0;
            if (computeData.isTranslucent)
            {
                ShadowComputeData shadowData2 = shadowData;
                shadowData2.normal = -shadowData2.normal;
                surfaceShadow2 = CalculateShadowFactorWithBlend(dirLights[i], i, shadowData2, shadowMap, shadowSampler, cascadeShadow[i], PCF_KERNEL_SIZE);
            }
            
#ifdef DEBUG_SHADOWS
            lightingResult.diffuse = surfaceShadow;
            return lightingResult;
#endif
            lightingResult.diffuse *= surfaceShadow;
            lightingResult.specular *= surfaceShadow;
            
            thinLighting.specular *= surfaceShadow2;
            thinLighting.diffuse *= surfaceShadow2;
        }

        totalResult.diffuse += lightingResult.diffuse + thinLighting.diffuse * THIN_TRANSLUCENCY_FACTOR;
        totalResult.specular += lightingResult.specular + thinLighting.specular * THIN_TRANSLUCENCY_FACTOR;
    }

    // Spot lights
    for (i = 0; i < computeData.nbSpot; ++i)
    {
        if (!spotLights[i].isActive)
            continue;
        
        const LightingResult lightingResult = ComputeSpotLighting(spotLights[i], computeData);
        totalResult.diffuse += lightingResult.diffuse;
        totalResult.specular += lightingResult.specular;
    }

    // Point lights
    for (i = 0; i < computeData.nbPoint; ++i)
    {
        if (!pointLights[i].isActive)
            continue;
        
        const LightingResult lightingResult = ComputePointLighting(pointLights[i], computeData);
        totalResult.diffuse += lightingResult.diffuse;
        totalResult.specular += lightingResult.specular;
    }

    totalResult.diffuse = saturate(totalResult.diffuse);
    totalResult.specular = saturate(totalResult.specular);

    return totalResult;
}

#endif
