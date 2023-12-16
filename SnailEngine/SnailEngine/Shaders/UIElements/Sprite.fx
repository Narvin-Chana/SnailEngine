#include "UIElement.hlsl"

Texture2D Sprite;
SamplerState SpriteSampler;

struct SpritePixelIn
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

#ifdef INFINITE_UV
static const float infiniteUVScaling = 1000;
#endif

SpritePixelIn SpriteVS(uint vertexId : SV_VertexID)
{
    SpritePixelIn output;
    
    Rect normCorners = GetNormalizedCorners();
    
    float2 coord = GetVertexCoord(vertexId, normCorners);
#ifdef INFINITE_UV
    coord *= infiniteUVScaling;
#endif
    
    output.position = float4(coord, 1, 1);
    output.uv = uvScreenSpace[vertexId];
    
#ifdef INFINITE_UV
    output.uv = output.uv * infiniteUVScaling - (infiniteUVScaling / 2 - 0.5f);
#endif
    return output;
}

float4 SpritePS(SpritePixelIn input) : SV_Target
{
    return Sprite.Sample(SpriteSampler, input.uv);
}

technique11 SpriteTech
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, SpriteVS()));
        SetPixelShader(CompileShader(ps_5_0, SpritePS()));
    }
}