#include "stdafx.h"
#include "Texture.h"

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"
#include "Util/Util.h"

namespace Snail
{
void Texture::InitSampler()
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    DX_CALL(
        renderDevice->GetD3DDevice()->CreateSamplerState(&samplerDesc, &samplerState),
        DXE_ERROR_CREATE_SAMPLER_STATE
    );
}

void Texture::SetSampler(const D3D11_SAMPLER_DESC& samplerDesc)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();
    DX_RELEASE(samplerState);
    DX_CALL(
        renderDevice->GetD3DDevice()->CreateSamplerState(&samplerDesc, &samplerState),
        DXE_ERROR_CREATE_SAMPLER_STATE
    );
}

void Texture::RenderImGui()
{
#ifdef _IMGUI_
    D3D11_TEXTURE2D_DESC desc;
    rawTexture->GetDesc(&desc);

    ImGui::Text("Size: %d x %d", desc.Width, desc.Height);
    ImGui::Image(shaderResourceView, ImVec2(100, 100));
#endif
}

Vector2 Texture::GetDimensions() const
{
    D3D11_TEXTURE2D_DESC desc;
    rawTexture->GetDesc(&desc);
    return Vector2{ static_cast<float>(desc.Width), static_cast<float>(desc.Height) };
}

Texture::~Texture()
{
    DX_RELEASE(samplerState);
}
}
