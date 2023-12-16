cbuffer TransformMatrixes
{
    matrix matWorldViewProj;
}

TextureCube SkyboxTexture;
SamplerState SkyboxTextureSampler;

Texture2D<float3> VolumetricAccumulation;

cbuffer VolumetricDensityFactor
{
    float volumetricDensityFactor;
};

struct VertexIn
{
    float3 position : POSITION;
    float3 normal : NORMAL2;
    matrix modelMatrix : MODEL_MATRIX;
};

struct PixelIn
{
    float4 position : SV_Position;
    float3 normal : NORMAL2;
};

PixelIn SkyboxVS(VertexIn input)
{
    PixelIn output;
    output.position = mul(mul(float4(input.position, 1), input.modelMatrix), matWorldViewProj);
    output.position.z = 0;
    output.normal = input.normal;
    return output;
}

float4 SkyboxPS(PixelIn input) : SV_Target
{    
    float4 skyColor = SkyboxTexture.Sample(SkyboxTextureSampler, input.normal);
    float3 volumetricAccumulation = VolumetricAccumulation.Load(float3(input.position.xy, 0));
    return float4(skyColor.xyz * (1 - volumetricDensityFactor * volumetricAccumulation) + volumetricAccumulation * volumetricDensityFactor, 1);
}

technique11 Skybox
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, SkyboxVS()));
        SetPixelShader(CompileShader(ps_5_0, SkyboxPS()));
    }
}