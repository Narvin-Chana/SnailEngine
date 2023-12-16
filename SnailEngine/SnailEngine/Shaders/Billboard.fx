#include "ScreenSpaceDef.hlsli"

Texture2D Billboard;
SamplerState BillboardSampler;

cbuffer TransformMatrixes
{
    matrix matWorldViewProj;
};

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
    float2 uv : TEXCOORD;
};

struct PixelOut
{
    float4 albedo : SV_Target2;
    float4 unlit : SV_Target5;
};

PixelIn BillboardVS(VertexIn input)
{
    PixelIn output;
    
    output.position = mul(float4(input.position, 1), input.modelMatrix);
    output.uv = input.uv;
    return output;
}

PixelOut BillboardPS(PixelIn input)
{
    PixelOut pout;
    pout.albedo = Billboard.Sample(BillboardSampler, input.uv);
    
    // Perform alpha test on billboard pixel
    if (pout.albedo.w <= 0.05f)
    {
        discard;
    }
    pout.unlit = float4(1, 1, 1, 1);
    return pout;
}

technique11 BillboardTech
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, BillboardVS()));
        SetPixelShader(CompileShader(ps_5_0, BillboardPS()));
    }
}