cbuffer TransformMatrixes : register(b0)
{
    matrix matViewProj;
}

struct MaterialParameters
{
    float3 diffuse;
    float3 ambient; // Not used
    float3 emission;
    float3 specular;
    float specularExp; // shininess
    float2 uvScale;
    float2 primaryBlendUvScale;
    float2 secondaryBlendUvScale;
};

cbuffer MaterialParameters : register(b1)
{
    MaterialParameters material;
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
    float4 worldPosition : POSITION;
    float3 bitangent : NORMAL0;
    float3 tangent : NORMAL1;
    float3 normal : NORMAL2;
    float2 uv : TEXCOORD;
};

struct GBuffer
{
    // Extra params
    // .r = specular exponent
    // .g = isTranslucent
    // .b = TBD
    float3 extraParams : SV_Target0;
    float3 normal : SV_Target1;
    float3 albedo : SV_Target2;
    float3 specular : SV_Target3;
    float3 emission : SV_Target4;
    float3 unlit : SV_Target5;
};

Texture2D Diffuse;
SamplerState DiffuseSampler;

#ifdef TEXTURE_BLENDING
Texture2D PrimaryBlendDiffuse;
SamplerState PrimaryBlendDiffuseSampler;

Texture2D PrimaryBlend;
SamplerState PrimaryBlendSampler;

Texture2D SecondaryBlendDiffuse;
SamplerState SecondaryBlendDiffuseSampler;

Texture2D SecondaryBlend;
SamplerState SecondaryBlendSampler;
#endif

Texture2D NormalMap;
SamplerState NormalMapSampler;

PixelIn DeferredVS(VertexIn input)
{
    PixelIn output;
    matrix matWorldViewProj = mul(input.modelMatrix, matViewProj);
    output.position = mul(float4(input.position, 1), matWorldViewProj);
    
    // Normals
    output.tangent = normalize(mul(float4(normalize(input.tangent), 0), transpose(input.invModelMatrix)).xyz);
    output.normal = normalize(mul(float4(normalize(input.normal), 0), transpose(input.invModelMatrix)).xyz);
    output.bitangent = normalize(mul(float4(normalize(cross(output.normal, output.tangent)), 0), transpose(input.invModelMatrix)).xyz);
    
    output.uv = input.uv;
    return output;
}

GBuffer DeferredPS(PixelIn input, bool isFrontFace : SV_IsFrontFace)
{
    GBuffer gbuffer;
    
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
    float3 N = NormalMap.Sample(NormalMapSampler, input.uv).xyz * 2 - 1;
    
    float3 normal = normalize(mul(N, TBN));
    
    if (!isFrontFace)
        normal = -normal;
        
    gbuffer.normal = normal * 0.5 + 0.5;
    
    const float3 diffuse = Diffuse.Sample(DiffuseSampler, input.uv * material.uvScale).rgb;

#ifdef TEXTURE_BLENDING
    const float3 primaryDiffuse = PrimaryBlendDiffuse.Sample(PrimaryBlendDiffuseSampler, input.uv * material.uvScale).rgb;
    const float primaryBlend = PrimaryBlend.Sample(PrimaryBlendSampler, input.uv * material.primaryBlendUvScale).x;

    const float3 secondaryDiffuse = SecondaryBlendDiffuse.Sample(SecondaryBlendDiffuseSampler, input.uv * material.uvScale).rgb;
    const float secondaryBlend = SecondaryBlend.Sample(SecondaryBlendSampler, input.uv * material.secondaryBlendUvScale).x;
    
    const float3 firstBlendAlbedo = lerp(primaryDiffuse, diffuse, primaryBlend);
    const float3 finalAlbedo = lerp(secondaryDiffuse, firstBlendAlbedo, secondaryBlend);
    gbuffer.albedo = float4(material.diffuse, 1) * finalAlbedo;

#else 
    gbuffer.albedo = material.diffuse * diffuse;
#endif
    // Specular exponent
    gbuffer.extraParams.r = material.specularExp;
    
#ifdef THIN_TRANSLUCENCY
    gbuffer.extraParams.g = 1;
#endif
    
    gbuffer.specular = material.specular;
    gbuffer.emission = material.emission;
    gbuffer.unlit = 0;

    return gbuffer;
}

technique11 Deferred
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, DeferredVS()));
        SetPixelShader(CompileShader(ps_5_0, DeferredPS()));
    }
}