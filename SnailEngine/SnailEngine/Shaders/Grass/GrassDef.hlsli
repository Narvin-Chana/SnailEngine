#ifndef __GRASS_DEF_HLSL__
#define __GRASS_DEF_HLSL__

static const int GRASS_VERTEX_COUNT = 9;

static const float3 LOD_GRASS_COLOR = float3(0.65, 0.8, 0.25);

struct GrassInstanceData
{
	float3 worldPosition;
	float3 baseColor;
    float3 tipColor;
    float randomAngle;
    float randomShade;
    float randomHeight;
    float randomLean;
};

float2 getValueFromCubicBezierCurve(float t, float2 p0, float2 p1, float2 p2, float2 p3)
{
    return pow((1 - t), 3) * p0 + 3 * pow((1 - t), 2) * t * p1 + 3 * pow((1 - t), 2) * p2 + pow(t, 3) * p3;
}

float3 getValueFromCubicBezierCurve(float t, float3 p0, float3 p1, float3 p2, float3 p3)
{
    return pow((1 - t), 3) * p0 + 3 * pow((1 - t), 2) * t * p1 + 3 * pow((1 - t), 2) * p2 + pow(t, 3) * p3;
}

float invLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}

float remap(float v, float inMin, float inMax, float outMin, float outMax)
{
    float t = invLerp(inMin, inMax, v);
    return lerp(outMin, outMax, t);
}

float decayWithFunction(float val, float x1, float x2)
{
    float a = 1 / (x1 - x2);
    float b = -a * x2;
    return saturate(a * val + b);
}

#endif