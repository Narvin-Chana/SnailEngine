#include "stdafx.h"
#include "VolumetricLighting.h"

#include <algorithm>
#include <random>

#include "D3D11Device.h"
#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"
#include "Rendering/Shaders/ComputeShader.h"
#include "Rendering/Shadows/DirectionalShadowMap.h"

namespace Snail
{

VolumetricLighting::VolumetricLighting(D3D11Device* device)
    : halfResAccumulationDataBuffer(D3D11Buffer::CreateConstantBuffer<HalfResAccumulationData>())
    , renderDevice(device)
{
    const DirectX::XMINT2 res = renderDevice->GetResolutionSize();
    InitTextures(res.x, res.y);
    ReloadShaders();
}

VolumetricLighting::~VolumetricLighting()
{
    DX_RELEASE(finalAccumulationUAV);
    DX_RELEASE(halfResAccumulationUAV);
    DX_RELEASE(halfResDepthUAV);

    DX_RELEASE(comparisonFilteringSampler);
    DX_RELEASE(pointFilteringSampler);
    DX_RELEASE(bilinearFilteringSampler);
}

void VolumetricLighting::Render()
{
    if (isActive)
    {
        // Accumulation pass to render light geometry for each volumetric and execute ray marching
        RenderHalfResAccumulationPass();

        RenderHalfResDepthPass();

        // Gather pass to apply bilateral Gaussian Blur
        if (doBlur)
        {
            RenderBilateralGaussianBlurPass();
        }

        // Upscale Pass to generate final accumulation buffer
        RenderUpsamplePass();
    }
}

void VolumetricLighting::Resize(const long width, const long height)
{
    InitTextures(width, height);
}

void VolumetricLighting::Clear()
{
    const Color blackColor;
    renderDevice->GetImmediateContext()->ClearUnorderedAccessViewFloat(halfResAccumulationUAV, blackColor);
    renderDevice->GetImmediateContext()->ClearUnorderedAccessViewFloat(halfResDepthUAV, blackColor);
    renderDevice->GetImmediateContext()->ClearUnorderedAccessViewFloat(finalAccumulationUAV, blackColor);
}

void VolumetricLighting::ReloadShaders()
{
    halfResAccumulationPass = std::make_unique<ComputeShader>(L"SnailEngine/Shaders/VolumetricLighting/AccumulationPass.cs.hlsl");
    halfResDepthPass = std::make_unique<ComputeShader>(L"SnailEngine/Shaders/VolumetricLighting/DownsampleDepthPass.cs.hlsl");
    bilateralGaussianBlurPass = std::make_unique<ComputeShader>(L"SnailEngine/Shaders/VolumetricLighting/BilateralGaussianBlur.cs.hlsl");
    upsamplePass = std::make_unique<ComputeShader>(L"SnailEngine/Shaders/VolumetricLighting/UpsamplePass.cs.hlsl");
    std::random_device rd;
    std::mt19937 g(rd());
    std::iota(randomRayIndices.begin(), randomRayIndices.end(), 0);
    std::ranges::shuffle(randomRayIndices, g);
}

bool VolumetricLighting::IsActive() const noexcept
{
    return isActive;
}

Texture2D* VolumetricLighting::GetVolumetricAccumulationBuffer() const noexcept
{
    return finalAccumulationTexture.get();
}

void VolumetricLighting::InitTextures(const long width, const long height)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    // Final accumulation texture (full res)
    finalAccumulationTexture = std::make_unique<Texture2D>(desc, desc.Format);

    // Half res accumulation texture
    desc.Width = width / 2;
    desc.Height = height / 2;
    halfResAccumulationTexture = std::make_unique<Texture2D>(desc, desc.Format);

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = desc.Format;
    uavDesc.Texture2D.MipSlice = 0;

    DX_RELEASE(finalAccumulationUAV);
    DX_CALL(renderDevice->GetD3DDevice()->CreateUnorderedAccessView(finalAccumulationTexture->GetRawTexture(), &uavDesc, &finalAccumulationUAV),
        "Failed to create accumulation UAV");

    DX_RELEASE(halfResAccumulationUAV);
    DX_CALL(renderDevice->GetD3DDevice()->CreateUnorderedAccessView(halfResAccumulationTexture->GetRawTexture(), &uavDesc, &halfResAccumulationUAV),
        "Failed to create half resolution UAV");

    // Half res depth texture
    desc.Format = DXGI_FORMAT_R16_FLOAT;
    halfResDepthTexture = std::make_unique<Texture2D>(desc, desc.Format);

    uavDesc.Format = desc.Format;
    DX_RELEASE(halfResDepthUAV);
    DX_CALL(renderDevice->GetD3DDevice()->CreateUnorderedAccessView(halfResDepthTexture->GetRawTexture(), &uavDesc, &halfResDepthUAV),
        "Failed to create half resolution UAV");

    // Two samplers
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    DX_RELEASE(comparisonFilteringSampler);
    DX_CALL(renderDevice->GetD3DDevice()->CreateSamplerState(&samplerDesc, &comparisonFilteringSampler), DXE_ERROR_CREATE_SAMPLER_STATE);

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
    DX_RELEASE(pointFilteringSampler);
    DX_CALL(renderDevice->GetD3DDevice()->CreateSamplerState(&samplerDesc, &pointFilteringSampler), DXE_ERROR_CREATE_SAMPLER_STATE);

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
    DX_RELEASE(bilinearFilteringSampler);
    DX_CALL(renderDevice->GetD3DDevice()->CreateSamplerState(&samplerDesc, &bilinearFilteringSampler), DXE_ERROR_CREATE_SAMPLER_STATE);

}

void VolumetricLighting::RenderHalfResAccumulationPass()
{
    halfResAccumulationPass->BindComputedUAV(halfResAccumulationUAV);
    static RendererModule& rm = WindowsEngine::GetModule<RendererModule>();
    const Texture2D* depthTexture = rm.GetDirectionalShadowMap()->GetDepthTexture();
    halfResAccumulationPass->BindSRVAndSampler(0, depthTexture->GetShaderResourceView(), depthTexture->GetSamplerState());
    halfResAccumulationPass->BindSRV(1, renderDevice->GetDepthShaderResourceView());
    halfResAccumulationPass->BindSampler(1, comparisonFilteringSampler);

    const FixedVector<DirectionalLight, SceneData::MAX_DIR_LIGHTS>& dirLights = WindowsEngine::GetScene()->GetDirectionalLights();
    const D3D11Buffer& dirLightsBuffer = WindowsEngine::GetScene()->GetDirectionalLightsBuffer();
    const Camera* cam = WindowsEngine::GetCamera();
    HalfResAccumulationData data = {cam->GetProjectionMatrix().Invert().Transpose(), cam->GetViewMatrix().Invert().Transpose()};
    data.nbDirectional = static_cast<uint32_t>(dirLights.size());

    for (int i = 0; i < 16; ++i)
    {
        data.randomRayIndices[i].val1 = randomRayIndices[i];
        data.randomRayIndices[i].val2 = randomRayIndices[i + 1];
        data.randomRayIndices[i].val3 = randomRayIndices[i + 2];
        data.randomRayIndices[i].val4 = randomRayIndices[i + 3];
    }
    halfResAccumulationDataBuffer.UpdateData(data);

    halfResAccumulationPass->SetConstantBuffer(0, halfResAccumulationDataBuffer.GetBuffer());
    halfResAccumulationPass->SetConstantBuffer(1, dirLightsBuffer.GetBuffer());
    halfResAccumulationPass->SetConstantBuffer(2, rm.GetDirectionalShadowMap()->GetViewProjBuffer(dirLights).GetBuffer());

    halfResAccumulationPass->Bind();

    D3D11_TEXTURE2D_DESC desc;
    halfResAccumulationTexture->GetRawTexture()->GetDesc(&desc);
    const UINT width = desc.Width, height = desc.Height;

    const int numGroupsX = (width + 7) / 8;
    const int numGroupsY = (height + 7) / 8;
    halfResAccumulationPass->Execute(numGroupsX, numGroupsY, 1);
    halfResAccumulationPass->UnbindSRV(0);
    halfResAccumulationPass->UnbindSRV(1);
    halfResAccumulationPass->Unbind();
}

void VolumetricLighting::RenderHalfResDepthPass()
{
    halfResDepthPass->BindComputedUAV(halfResDepthUAV);
    halfResDepthPass->BindSRVAndSampler(0, renderDevice->GetDepthShaderResourceView(), pointFilteringSampler);
    halfResDepthPass->Bind();

    D3D11_TEXTURE2D_DESC desc;
    halfResDepthTexture->GetRawTexture()->GetDesc(&desc);
    const UINT width = desc.Width, height = desc.Height;

    const int numGroupsX = (width + 7) / 8;
    const int numGroupsY = (height + 7) / 8;
    halfResDepthPass->Execute(numGroupsX, numGroupsY, 1);
    halfResDepthPass->UnbindSRV(0);
    halfResDepthPass->Unbind();
}

void VolumetricLighting::RenderBilateralGaussianBlurPass()
{
    bilateralGaussianBlurPass->BindComputedUAV(halfResAccumulationUAV);
    bilateralGaussianBlurPass->BindSRV(0, halfResDepthTexture->GetShaderResourceView());
    bilateralGaussianBlurPass->Bind();

    D3D11_TEXTURE2D_DESC desc;
    halfResAccumulationTexture->GetRawTexture()->GetDesc(&desc);
    const UINT width = desc.Width, height = desc.Height;

    const int numGroupsX = (width + 7) / 8;
    const int numGroupsY = (height + 7) / 8;
    bilateralGaussianBlurPass->Execute(numGroupsX, numGroupsY, 1);
    bilateralGaussianBlurPass->UnbindSRV(0);
    bilateralGaussianBlurPass->Unbind();
}

void VolumetricLighting::RenderUpsamplePass()
{
    upsamplePass->BindSRVAndSampler(0, halfResDepthTexture->GetShaderResourceView(), bilinearFilteringSampler);
    upsamplePass->BindSRVAndSampler(1, halfResAccumulationTexture->GetShaderResourceView(), pointFilteringSampler);
    upsamplePass->BindSRV(2, renderDevice->GetDepthShaderResourceView());
    upsamplePass->BindComputedUAV(finalAccumulationUAV);
    upsamplePass->Bind();

    D3D11_TEXTURE2D_DESC desc;
    finalAccumulationTexture->GetRawTexture()->GetDesc(&desc);
    const UINT width = desc.Width, height = desc.Height;

    const int numGroupsX = (width + 7) / 8;
    const int numGroupsY = (height + 7) / 8;
    upsamplePass->Execute(numGroupsX, numGroupsY, 1);
    upsamplePass->UnbindSRV(0);
    upsamplePass->UnbindSRV(1);
    upsamplePass->UnbindSRV(2);
    upsamplePass->Unbind();
}

void VolumetricLighting::RenderImGui()
{
#ifdef _IMGUI_
    static RendererModule& rm = WindowsEngine::GetModule<RendererModule>();

    ImGui::SeparatorText("Volumetric Lighting");
    if (ImGui::Checkbox("Volumetric Lighting", &isActive))
    {
        if (isActive)
            rm.lightingPassShader->AddDefine(RendererModule::VOLUMETRIC_LIGHTS_DEFINE);
        else
            rm.lightingPassShader->RemoveDefine(RendererModule::VOLUMETRIC_LIGHTS_DEFINE);
    }

    if (isActive)
    {
        if (ImGui::Checkbox("Debug volumetric shadows", &debugVolumetricShadows))
        {
            if (debugVolumetricShadows)
                rm.lightingPassShader->AddDefine(RendererModule::VOLUMETRIC_LIGHTING_DEBUG_DEFINE);
            else
                rm.lightingPassShader->RemoveDefine(RendererModule::VOLUMETRIC_LIGHTING_DEBUG_DEFINE);
        }

        ImGui::Checkbox("Bilateral Gaussian Blur", &doBlur);

        ImGui::Text("HalfRes Accumulation Buffer: ");
        ImGui::Image(halfResAccumulationTexture->GetShaderResourceView(), ImVec2(200, 150));

        ImGui::Text("HalfRes Depth: ");
        ImGui::Image(halfResDepthTexture->GetShaderResourceView(), ImVec2(200, 150));

        ImGui::Text("Final Accumulation Buffer \n(after nearest depth-aware upsample): ");
        ImGui::Image(finalAccumulationTexture->GetShaderResourceView(), ImVec2(200, 150));
    }
#endif
}

}
