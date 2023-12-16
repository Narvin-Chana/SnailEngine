#include "UIElement.hlsl"

struct SpriteVertexIn
{
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct SpritePixelIn
{
    float4 position : SV_Position;
    float2 charPosition : TEXCOORD0;
    float2 charUV : TEXCOORD1;
};

struct CharacterData
{
    matrix anchorMatrix;
    float2 charPosition;
    float2 charSize;
};

cbuffer TextInfo 
{
    matrix parentTransform;
};

cbuffer Chars
{
    CharacterData chars[128];
};

Texture2D Text;
SamplerState TextSampler;

SpritePixelIn TextVS(SpriteVertexIn input)
{
    SpritePixelIn output;
    
    CharacterData char = chars[input.instanceId];

    Rect rec = GetRectBounds(mul(float4(0, 0, 1, 1), char.anchorMatrix).xy, char.charSize);
    rec = TransformRect(rec, parentTransform);
    rec = TransformRect(rec, screenToNormMatrix);

    output.position = float4(GetVertexCoord(input.vertexId, rec), 1, 1);
    output.charUV = uvScreenSpace[input.vertexId] * char.charSize;
    output.charPosition = char.charPosition;
    return output;
}

float4 TextPS(SpritePixelIn input) : SV_Target
{
    int2 dim;
    int mip;
    Text.GetDimensions(0, dim.x, dim.y, mip);
    return Text.Sample(TextSampler, input.charPosition / dim + input.charUV / dim);
}

technique11 TextTech
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, TextVS()));
        SetPixelShader(CompileShader(ps_5_0, TextPS()));
    }
}