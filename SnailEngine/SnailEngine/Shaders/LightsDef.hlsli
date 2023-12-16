#ifndef __LIGHTS_DEF_HLSL__
#define __LIGHTS_DEF_HLSL__

static const int PCF_KERNEL_SIZE = 10;

struct LightComputeData
{
    uint nbDirectional;
    uint nbSpot;
    uint nbPoint;
    float3 cameraPosition;
    float3 posViewSpace;
    float3 worldPosition;
    float3 normal;
    float specularExp;
    float3 screenSpacePos;
    matrix viewMatrix;
    matrix invViewMatrix;
    bool isTranslucent;
    bool enableVolumetric;
};

struct LightingResult
{
    float3 diffuse;
    float3 specular;
};

struct DirectionalLight
{
    float3 Color;
    bool castsShadows;
    float3 Direction;
    bool isActive;
};

static const uint MAX_DIR_LIGHTS = 2;
static const uint MAX_SPOT_LIGHTS = 10;
static const uint MAX_POINT_LIGHTS = 40;

struct SpotLight
{
    float3 Position;
    float3 Color;
    float3 Direction;
    float3 Coefs; // constant, linear, quadratic
    float InnerConeAngle;
    float OuterConeAngle;
    bool isActive;
};

struct PointLight
{
    float3 Position;
    float3 Color;
    float3 Coefs; // constant, linear, quadratic
    bool isActive;
};

#endif