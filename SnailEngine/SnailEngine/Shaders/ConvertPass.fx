#include "ScreenSpaceDef.hlsli"

float4 VSMain(uint vertexId : SV_VertexID) : SV_Position
{
    return float4(vertexScreenSpace[vertexId], 1, 1); // Depth at 1 and is point so w=1
}

Texture2D UAV;
SamplerState UAVSampler;

float4 PSMain(float4 position : SV_Position) : SV_TARGET
{
    // Third coordinate is the mipmap level...
    return UAV.Load(float3(position.xy, 0));
}

technique11 ConvertPass
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, VSMain()));
        SetPixelShader(CompileShader(ps_5_0, PSMain()));
    }
}