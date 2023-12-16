#include "GrassDef.hlsli"
#include "../CommonMath.hlsli"
#include "../Noise.hlsl"

StructuredBuffer<GrassInstanceData> grassInstanceData;

cbuffer GrassParams
{
    matrix matWorldViewProj;
    matrix matWorld;
    matrix matView;
    matrix matViewInv;
    float time;
};

cbuffer TransformMatrixes
{
    matrix matViewProj;
}

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    uint instanceID : SV_InstanceID;
    uint vertexID : SV_VertexID;
};

struct PSInput
{
    float4 position : SV_Position;
    bool shouldDiscard : DISCARD_GRASS;
};

PSInput GrassVS(VSInput input)
{    
    GrassInstanceData gid = grassInstanceData[input.instanceID];
    
    PSInput output = (PSInput) 0;

    if (gid.randomHeight == 0)
    {
        output.shouldDiscard = true;
        return output;
    }
    
    // Add with instance world position to get final model position
    float3 finalWorldPosition = gid.worldPosition + input.position;
    float4 worldPosition = mul(float4(finalWorldPosition, 1), matWorld);
    float4 worldPositionVS = mul(worldPosition, matView);

    if (worldPositionVS.z > 400)
    {
        output.shouldDiscard = true;
        return output;
    }
    
    // If player is far away enough from the grass, then don't animate it
    float animateGrass = decayWithFunction(worldPositionVS.z, 50, 300);

    float leanAnimation = 0;
    if (animateGrass > 0.05)
    {
        leanAnimation = noise12((time * 0.35).xx + gid.worldPosition.xz * 137.423) * 0.1 * animateGrass;
    }
    float easedHeight = pow(input.uv.y, 2) * gid.randomHeight;
    float curveAmount = -1 * (gid.randomLean + leanAnimation) * easedHeight;
    
    float3x3 rotMatrixX = CreateRotationMatrixX(curveAmount);
    float3x3 rotMatrixY = CreateRotationMatrixY(gid.randomAngle);
    
    // Rotate finalWorldPosition within model space
    float3x3 grassMat = mul(rotMatrixY, rotMatrixX);
        
    if (animateGrass > 0.05)
    {
        // Simulate wind
        float windDir = noise12(finalWorldPosition.xz * 0.05 + 0.15 * time);
        float windNoiseSample = noise12(finalWorldPosition.xz * 0.25 + time * 1.0);
        float windLeanAngle = remap(windNoiseSample, -1.0, 1.0, 0.55, 1.0);
        windLeanAngle = pow(windLeanAngle, 2.0) * 0.35;
        float3 windAxis = float3(cos(windDir), 0.0, sin(windDir));
        windLeanAngle *= input.uv.y;
        grassMat = mul(rotateAxis(windAxis, windLeanAngle), grassMat);
    }
    
    // Add random height for more randomness
    input.position.y *= gid.randomHeight;
    
    // Add grass blade world position to the position of this vertex
    float3 finalPosition = mul(input.position, grassMat) + gid.worldPosition;
    output.position = mul(float4(finalPosition, 1), mul(matWorld, matViewProj));
    output.shouldDiscard = false;
    
    return output;
}

void GrassPS(PSInput position)
{
    // Discard if invalid grass blade to avoid writing to depth
    if (position.shouldDiscard)
    {
        discard;
    }
}

technique11 Deferred
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, GrassVS()));
        SetPixelShader(CompileShader(ps_5_0, GrassPS()));
    }
}