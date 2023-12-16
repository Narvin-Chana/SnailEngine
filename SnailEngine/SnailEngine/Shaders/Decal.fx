#include "CommonMath.hlsli"

cbuffer TransformMatrixes : register(b0)
{
    matrix matViewProj;
}

cbuffer InvViewMatrixes : register(b1)
{
    matrix invViewMatrix;
    matrix invProjectionMatrix;
}

struct VertexIn
{
    float3 position : POSITION;
    float3 bitangent : NORMAL0;
    float3 tangent : NORMAL1;
    float3 normal : NORMAL2;
    float2 uv : TEXCOORD;
    matrix modelMatrix : MODEL_MATRIX;
    matrix invModelMatrix : INV_MODEL_MATRIX;
};

struct PixelIn
{
    float4 position : SV_Position;
    matrix invModelMatrix : INV_MODEL_MATRIX;
};

struct GBuffer
{
    float3 albedo : SV_Target2;
    float3 unlit : SV_Target5;
};

Texture2D Diffuse;
SamplerState DiffuseSampler;

Texture2D DepthTexture;

PixelIn DecalVS(VertexIn input)
{
    PixelIn output;
    matrix matWorldViewProj = mul(input.modelMatrix, matViewProj);
    output.position = mul(float4(input.position, 1), matWorldViewProj);
    output.invModelMatrix = input.invModelMatrix;
    return output;
}

GBuffer DecalPS(PixelIn input)
{
    GBuffer gbuffer;
          
    float2 resolution;
    DepthTexture.GetDimensions(resolution.x, resolution.y);
    
    // Get depth at this pixel
    float2 depthCoord = input.position.xy / resolution;
    float depth = DepthTexture.Load(float3(input.position.xy, 0)).r;
    
    // Obtain position of object at depth
    float3 screenPosition = float3((depthCoord.x * 2 - 1), ((1 - depthCoord.y) * 2 - 1), depth);
    float4 viewPosition = mul(float4(screenPosition, 1), invProjectionMatrix);
    float4 worldPosition = mul(viewPosition, invViewMatrix);
    float4 objectPosition = mul(worldPosition, input.invModelMatrix);
    float3 localPosition = objectPosition.xyz / objectPosition.w;
    
    // localPosition gives us a position inside (or not) of our 1x1x1 cube centered at (0, 0, 0).
    // Reject anything outside.
    clip(0.5 - abs(localPosition.xyz));

    // Add 0.5 to get our texture coordinates [-0.5, 0.5] -> [0, 1].
    float2 decalTexCoord = localPosition.xy + 0.5;
    decalTexCoord.y = 1 - decalTexCoord.y;
    float4 color = Diffuse.Sample(DiffuseSampler, decalTexCoord);
    
    if (color.w <= 0.05f)
    {
        discard;
    }
    
    gbuffer.albedo = color.xyz;
    gbuffer.unlit = 0;

    return gbuffer;
}

technique11 Decal
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, DecalVS()));
        SetPixelShader(CompileShader(ps_5_0, DecalPS()));
    }
}