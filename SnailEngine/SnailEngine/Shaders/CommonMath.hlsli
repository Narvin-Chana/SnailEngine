#ifndef __COMMON_MATH_HLSL__
#define __COMMON_MATH_HLSL__

#define MATHS_PI 3.14159265359

float3x3 rotateAxis(float3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return float3x3(
    oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
  );
}

float3x3 CreateRotationMatrixX(float theta)
{
    float3x3 mat =
    {
        1,      0,          0,
        0, cos(theta), -sin(theta),
        0, sin(theta),  cos(theta)
    };
    return mat;
}

float3x3 CreateRotationMatrixY(float theta)
{
    float3x3 mat =
    {
         cos(theta), 0, sin(theta),
              0    , 1,      0,
        -sin(theta), 0,  cos(theta)
    };
    return mat;
}

float3x3 CreateRotationMatrixZ(float theta)
{
    float3x3 mat =
    {
        cos(theta), -sin(theta), 0,
        sin(theta),  cos(theta), 0,
             0,          0,      1
    };
    return mat;
}

float3 DepthToWorld(Texture2D depthTexture, matrix inverseProjMatrix, float2 pos, float2 uv)
{
    float depth = depthTexture.Load(float3(pos, 0)).r;
    float3 posClip = float3(uv * 2 - 1, depth);
    float4 worldPos = mul(float4(posClip, 1), inverseProjMatrix);
    return (worldPos / worldPos.w).xyz;
}

#endif