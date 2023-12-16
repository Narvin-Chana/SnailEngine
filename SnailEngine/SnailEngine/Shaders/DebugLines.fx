cbuffer TransformMatrixes
{
    matrix matViewProj;
}

struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PixelIn
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct PixelOut
{
    float4 albedo : SV_Target2;
    float4 unlit : SV_Target5;
};

PixelIn DebugLinesVS(VertexIn input)
{
    PixelIn output;
    output.position = mul(matViewProj, float4(input.position, 1));
    output.color = input.color;
    return output;
}

PixelOut DebugLinesPS(PixelIn input)
{
    PixelOut pout;
    pout.albedo = input.color;
    pout.unlit = float4(1, 1, 1, 1);
    return pout;
}

technique11 DebugLines
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, DebugLinesVS()));
        SetPixelShader(CompileShader(ps_5_0, DebugLinesPS()));
    }
}