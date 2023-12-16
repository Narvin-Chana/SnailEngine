#include "Lights.hlsl"
#include "ScreenSpaceDef.hlsli"
#include "CommonMath.hlsli"

cbuffer SceneInfo
{
    matrix viewMat;
    matrix invViewMatrix;
    matrix invViewProjMatrix;
    float3 cameraPosition;
    uint nbDirectional;
    uint nbSpot;
    uint nbPoint;
    float volumetricDensityFactor;
}

cbuffer PostProcess
{
    bool ssaoEnabled;
};

cbuffer DirectionalLights
{
    DirectionalLight dirLights[MAX_DIR_LIGHTS];
}

cbuffer DirectionalLightShadows
{
    CascadeShadows dirLightMatrix[MAX_DIR_LIGHTS];
}

cbuffer SpotLights
{
    SpotLight spotLights[MAX_SPOT_LIGHTS];
}

cbuffer PointLights
{
    PointLight pointLights[MAX_POINT_LIGHTS];
}

static const float globalAmbient = 0.3;

Texture2DArray GBuffer;
SamplerState GBufferSampler;

Texture2DArray DirectionalShadowMap;
SamplerState DirectionalShadowMapSampler;

Texture2D DepthTexture;
Texture2D SSAOTexture;

#ifdef VOLUMETRIC_LIGHTING
Texture2D<float3> VolumetricAccumulationBuffer;
#endif

struct VSInput
{
    uint vertexId : SV_VertexID;
};

struct VSOutput
{
    float2 uv : TEXCOORD;
    float4 pos : SV_Position;
};

VSOutput LightingVS(VSInput input)
{
    VSOutput output;
    output.uv = uvScreenSpace[input.vertexId];
    // Depth at 1 and is point so w=1
    output.pos = float4(vertexScreenSpace[input.vertexId], 1, 1);
    return output;
}

float4 LightingPS(VSOutput input) : SV_Target
{
    float2 pos = input.pos.xy;
    
    LightComputeData computeData;
    
    float ssao = 1;
    if (ssaoEnabled)
        ssao = saturate(SSAOTexture.Sample(GBufferSampler, input.uv).r);
        
    // Rebuild world pos from depth
    input.uv.y = 1 - input.uv.y;
    computeData.worldPosition = DepthToWorld(DepthTexture, invViewProjMatrix, pos, input.uv);
    
    computeData.enableVolumetric = volumetricDensityFactor > 0.001;
    
    float4 posView = mul(float4(computeData.worldPosition, 1), viewMat);
    computeData.posViewSpace = posView.xyz;
    computeData.viewMatrix = viewMat;
    computeData.invViewMatrix = invViewMatrix;
    computeData.cameraPosition = cameraPosition;
    computeData.normal = normalize(GBuffer.Load(float4(pos, 1, 0)).rgb * 2 - 1);
    
    computeData.nbDirectional = nbDirectional;
    computeData.nbSpot = nbSpot;
    computeData.nbPoint = nbPoint;
    
    float3 extraParams = GBuffer.Load(float4(pos, 0, 0));
    computeData.specularExp = extraParams.r;
    computeData.isTranslucent = extraParams.g == 1;
    
    computeData.screenSpacePos = input.pos.xyz;

    const float3 albedo = GBuffer.Load(float4(pos, 2, 0)).rgb;

    float unlit = GBuffer.Load(float4(pos, 5, 0)).r;
    if (unlit == 1)
    {
        return float4(albedo, 1);
    }

    // Calculate lighting result via BlinnPhong
    const LightingResult lit = ComputeAllLighting(computeData, dirLights, spotLights, pointLights, DirectionalShadowMap, DirectionalShadowMapSampler, dirLightMatrix);
    
#ifdef DEBUG_SHADOWS
    return float4(lit.diffuse, 1);
#endif

#ifdef VOLUMETRIC_LIGHTING
        float3 volumetricAccumulation = VolumetricAccumulationBuffer.Load(float3(pos, 0));
#endif
    
#if defined(VOLUMETRIC_LIGHTING) && defined(DEBUG_VOLUMETRIC) 
    return float4(volumetricAccumulation, 1);
#endif
    
    const float3 surfaceColor = albedo * lit.diffuse;
    
#ifdef VOLUMETRIC_LIGHTING
    const float3 diffuse = surfaceColor * (1 - volumetricDensityFactor * volumetricAccumulation) + volumetricAccumulation * volumetricDensityFactor;
#else
    const float3 diffuse = surfaceColor;
#endif
    
    const float3 ambient = albedo * globalAmbient * ssao;
    const float3 specular = GBuffer.Load(float4(pos, 3, 0)).rgb * lit.specular;
    const float3 emission = GBuffer.Load(float4(pos, 4, 0)).rgb * albedo;
    
    float3 finalColor = emission + ambient + diffuse + specular;
    
    return float4(finalColor, 1);
}

technique11 Lighting
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, LightingVS()));
        SetPixelShader(CompileShader(ps_5_0, LightingPS()));
    }
}