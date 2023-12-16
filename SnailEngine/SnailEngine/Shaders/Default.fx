#include "ScreenSpaceDef.hlsli"

float4 DefaultVS(uint vertexId : SV_VertexID) : SV_Position
{
    return float4(vertexScreenSpace[vertexId], 1, 1); // Depth at 1 and is point so w=1
}

cbuffer IndexGBuffer
{
    int index;
};

Texture2DArray GBuffer;
SamplerState GBufferSampler;

float4 DefaultPS(float4 position : SV_Position) : SV_Target
{
    return GBuffer.Load(float4(position.xy, index, 0));
}

technique11 Deferred
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVS()));
        SetPixelShader(CompileShader(ps_5_0, DefaultPS()));
    }
}