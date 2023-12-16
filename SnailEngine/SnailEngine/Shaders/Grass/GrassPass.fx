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
    float2 uv : TEXCOORD;
    float3 normal1 : NORMAL0;
    float3 normal2 : NORMAL1;
    uint instanceID : SV_InstanceID;
    float depth : DEPTH;
};

struct GBuffer
{
    float3 extraParams : SV_Target0;
    float3 normal : SV_Target1;
    float3 albedo : SV_Target2;
    float3 specular : SV_Target3;
    float3 emission : SV_Target4;
    float3 unlit : SV_Target5;
};

PSInput GrassVS(VSInput input)
{
    PSInput output = (PSInput)0;
    
    GrassInstanceData gid = grassInstanceData[input.instanceID];
    
    // If not valid grass then set all verts to 0 to not render grass (vertex degeneracy)
    if (gid.randomHeight == 0)
    {
        output.instanceID = input.instanceID;
        return output;
    }
    
    // Add with instance world position to get final model position
    float3 finalWorldPosition = gid.worldPosition + input.position;
    float4 worldPosition = mul(float4(finalWorldPosition, 1), matWorld);
    
    // If player is far away enough from the grass, then don't show it at all with a certain probability
    float4 worldPositionVS = mul(worldPosition, matView);
    
    if (worldPositionVS.z > 400)
    {
        output.position = float4(-10, -10, -10, 1);
        output.instanceID = input.instanceID;
        return output;
    }
    
    // If player is far away enough from the grass, then don't animate it
    float animateGrass = decayWithFunction(worldPositionVS.z, 50, 150);
    
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
        // Simulate wind if animated
        float windDir = noise12(finalWorldPosition.xz * 0.05 + 0.15 * time);
        float windNoiseSample = noise12(finalWorldPosition.xz * 0.25 + time * 1.0);
        float windLeanAngle = remap(windNoiseSample, -1.0, 1.0, 0.55, 1.0);
        windLeanAngle = pow(windLeanAngle, 2.0) * 0.35;
        float3 windAxis = float3(cos(windDir), 0.0, sin(windDir));
        windLeanAngle *= input.uv.y;
        grassMat = mul(rotateAxis(windAxis, windLeanAngle), grassMat);
    }
    
    // Adjust normal depending on the curve of the blade
    float ncurve1 = -gid.randomLean * easedHeight;
    float3 n1 = float3(0.0, (input.uv.y + 0.01), 0.0);
    n1 = mul(n1, CreateRotationMatrixX(ncurve1));
    float ncurve2 = -gid.randomLean * easedHeight * 0.9;
    float3 n2 = float3(0.0, (input.uv.y + 0.01) * 0.9, 0.0);
    n2 = mul(n2, CreateRotationMatrixX(ncurve2));

    float3 ncurve = normalize(n1 - n2);
    
    float3 grassFaceNormal = float3(0.0, 0.0, 1.0);
    grassFaceNormal = mul(grassFaceNormal, grassMat);
    
    // Flip normal depending on camera direction
    float4 normalVS = mul(float4(grassFaceNormal, 0), matViewInv);
    float4 viewDir = -worldPositionVS;
    float viewDotNormal = dot(viewDir, normalVS);
    float flipNormal = viewDotNormal < 0 ? -1 : 1;
        
    // Add random height for more randomness
    input.position.y *= gid.randomHeight;

    // Add grass blade world position to the position of this vertex
    float3 finalPosition = mul(input.position, grassMat) + gid.worldPosition;
    output.position = mul(float4(finalPosition, 1), matWorldViewProj);
    
    // Generate two normals, one on each side of the blade
    float3 grassVertexNormal = float3(0.0, -ncurve.z, ncurve.y);
    float3 grassVertexNormal1 = mul(grassVertexNormal * flipNormal, CreateRotationMatrixY(MATHS_PI * 0.4 * flipNormal));
    float3 grassVertexNormal2 = mul(grassVertexNormal * flipNormal, CreateRotationMatrixY(MATHS_PI * -0.4 * flipNormal));
    grassVertexNormal1 = mul(grassVertexNormal1, grassMat);
    grassVertexNormal2 = mul(grassVertexNormal2, grassMat);
    output.normal1 = grassVertexNormal1;
    output.normal2 = grassVertexNormal2;
    
    output.uv = input.uv;
    output.depth = worldPositionVS.z;
    output.instanceID = input.instanceID;
    
    return output;
}

GBuffer GrassPS(const PSInput input)
{
    GrassInstanceData gid = grassInstanceData[input.instanceID];
    
    // Write to GBuffer (worldPos, normals, specular, etc...)
    GBuffer gBuffer;
    gBuffer.extraParams = float3(16, 0, 0);

    // Decay the color and normals with distance
    float lodDecayValue = decayWithFunction(input.depth, 75, 350);
    
    // Lerp between the two extremity normals using the current width as scaling to give a 3d effect
    // After a certain distance, only use a default normal to avoid noise
    gBuffer.normal = lerp(float3(0, -1, 0), normalize(lerp(input.normal1, input.normal2, input.uv.x)), lodDecayValue);

    // Calculate final pixel color
    float grassMiddle = smoothstep(abs(input.uv.x), 0.0, 0.1);
    float3 diffuse = lerp(gid.baseColor, gid.tipColor, pow(input.uv.y, 2)) * gid.randomShade;
    diffuse *= lerp(0.85, 1.0, grassMiddle);
    
    // Decay color with distance
    gBuffer.albedo = lerp(LOD_GRASS_COLOR, diffuse, lodDecayValue);
    
    gBuffer.specular = float3(0.5, 0.5, 0.5);
    gBuffer.emission = 0;
    gBuffer.unlit = 0;
    
    return gBuffer;
}

technique11 Deferred
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, GrassVS()));
        SetPixelShader(CompileShader(ps_5_0, GrassPS()));
    }
}