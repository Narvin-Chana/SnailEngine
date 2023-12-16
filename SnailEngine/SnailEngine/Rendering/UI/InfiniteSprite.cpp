#include "stdafx.h"
#include "InfiniteSprite.h"

#include "SpriteVertex.h"
#include "Rendering/Texture2D.h"

namespace Snail
{

InfiniteSprite::InfiniteSprite(const Params& params)
    : Sprite{params}
{
    D3D11_SAMPLER_DESC dsc;
    ZeroMemory(&dsc, sizeof(dsc));
    dsc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
    dsc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    dsc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    dsc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    dsc.MipLODBias = 0;
    dsc.MaxAnisotropy = 1;
    dsc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    dsc.BorderColor[0] = 0;
    dsc.BorderColor[1] = 0;
    dsc.BorderColor[2] = 0;
    dsc.BorderColor[3] = 0;
    dsc.MinLOD = 0;
    dsc.MaxLOD = D3D11_FLOAT32_MAX;
    texture->SetSampler(dsc);
}
void InfiniteSprite::InitShaders()
{
    drawEffect = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/UIElements/Sprite.fx", SpriteVertex::layout, SpriteVertex::elementCount, std::unordered_set<std::string>{ INFINITE_UV_MACRO });
}

}
