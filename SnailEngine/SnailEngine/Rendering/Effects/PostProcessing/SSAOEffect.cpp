#include "stdafx.h"
#include "SSAOEffect.h"

#include <random>

#include "Core/WindowsEngine.h"
#include "Rendering/D3D11Device.h"

namespace Snail
{

    SSAOEffect::SSAOEffect(D3D11Device* device)
        : PostProcessEffect(L"SnailEngine/Shaders/PostProcessing/SSAO.cs.hlsl")
        , renderDevice(device)
    {
        CalculateRotationNoiseTexture();
        CalculateNoiseData();
        const DirectX::XMINT2 res = device->GetResolutionSize();
        ResizeTexture(res.x, res.y);

        blur.SetActive(true);
    }

    SSAOEffect::~SSAOEffect()
    {
        DX_RELEASE(outputUAV);
        DX_RELEASE(outputTexture);
        DX_RELEASE(outputSRV);
    }

    ID3D11Texture2D* SSAOEffect::GetSSAOTexture()
    {
        return outputTexture;
    }

    ID3D11UnorderedAccessView* SSAOEffect::GetSSAOUAV()
    {
        return outputUAV;
    }

    ID3D11ShaderResourceView* SSAOEffect::GetSSAOSRV()
    {
        return outputSRV;
    }


    void SSAOEffect::RenderEffect(D3D11Device*)
    {
        RenderEffect(renderDevice, outputTexture, outputUAV);
    }

    void SSAOEffect::CalculateRotationNoiseTexture()
    {
        // Inspired by learnopenGL ssao
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
        std::default_random_engine generator;
        std::vector<Vector3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++)
        {
            Vector3 noise(
                randomFloats(generator) * 2.0f - 1.0f,
                randomFloats(generator) * 2.0f - 1.0f,
                0.0f);
            ssaoNoise.push_back(noise);
        }

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof desc);
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.Width = 4;
        desc.Height = desc.Width;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = ssaoNoise.data();
        initData.SysMemPitch = static_cast<UINT>(ssaoNoise.size() / 4 * sizeof(Vector3));

        noiseTexture = std::make_unique<Texture2D>(desc, DXGI_FORMAT_R16G16B16A16_FLOAT, initData);

        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof samplerDesc);
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        noiseTexture->SetSampler(samplerDesc);
    }

    void SSAOEffect::CalculateNoiseData()
    {
        // Inspired by learnopenGL ssao
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
        std::default_random_engine generator;
        std::vector<Vector3> ssaoKernel;
        for (unsigned int i = 0; i < 64; ++i)
        {
            Vector3 sample(
                randomFloats(generator) * 2.0f - 1.0f,
                randomFloats(generator) * 2.0f - 1.0f,
                randomFloats(generator)
            );
            sample.Normalize(sample);
            sample *= randomFloats(generator);

            float scale = static_cast<float>(i) / 64.0f;
            scale = std::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel.push_back(sample);
        }

        std::ranges::transform(ssaoKernel, data.samples.begin(), [](const auto& vec3)
        {
            return Vector4{ vec3.x, vec3.y, vec3.z, 0 };
        });
    }

    void SSAOEffect::ResizeTexture(const long width, const long height)
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

        DX_RELEASE(outputTexture);
        DX_CALL(renderDevice->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &outputTexture), "Failed to create SSAO texture");

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Format = desc.Format;
        uavDesc.Texture2D.MipSlice = 0;

        DX_RELEASE(outputUAV);
        DX_CALL(renderDevice->GetD3DDevice()->CreateUnorderedAccessView(outputTexture, &uavDesc, &outputUAV), "Failed to create SSAO UAV");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format = desc.Format;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;

        DX_RELEASE(outputSRV);
        DX_CALL(renderDevice->GetD3DDevice()->CreateShaderResourceView(outputTexture, &srvDesc, &outputSRV), "Failed to create SSAO SRV");
    }

    void SSAOEffect::RenderEffect(D3D11Device*, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output)
    {
        if (isActive)
        {
            static auto& cm = WindowsEngine::GetModule<CameraManager>();

            const Camera* cam = cm.GetCurrentCamera();

            // Get the dimensions of the UAV texture
            D3D11_TEXTURE2D_DESC desc;
            tex->GetDesc(&desc);
            const UINT width = desc.Width, height = desc.Height;

            data.screenSize = DirectX::XMINT2{ static_cast<int>(width), static_cast<int>(height) };
            data.invProjMat = cam->GetProjectionMatrix().Invert().Transpose();
            data.projMat = cam->GetProjectionMatrix().Transpose();
            data.invViewMat = cam->GetViewMatrix().Invert();
            dataBuffer.UpdateData(data);

            computeShader->SetConstantBuffer("SSAOParams", dataBuffer.GetBuffer());

            computeShader->BindComputedUAV(output);

            computeShader->BindSRV(0, renderDevice->GetGBufferTexture2DArray()->GetShaderResourceView());
            computeShader->BindSRV(1, renderDevice->GetDepthShaderResourceView());
            computeShader->BindSRV(2, noiseTexture->GetShaderResourceView());
            computeShader->BindSampler(0, noiseTexture->GetSamplerState());
            computeShader->Bind();

            
            // Determine the number of thread groups based on the dimensions
            const int numGroupsX = (width + 7) / 8;
            const int numGroupsY = (height + 7) / 8;
            computeShader->Execute(numGroupsX, numGroupsY, 1);
            computeShader->UnbindSRV(0);
            computeShader->UnbindSRV(1);
            computeShader->UnbindSRV(2);
            computeShader->Unbind(); // Could put this in execute...

            blur.RenderEffect(renderDevice, tex, output);
        }
    }

    void SSAOEffect::Resize(const long width, const long height)
    {
        ResizeTexture(width, height);
    }

    void SSAOEffect::RenderImGui()
    {
#ifdef _IMGUI_
        ImGui::SeparatorText("SSAO");

        ImGui::Checkbox("Active##SSAO", &isActive);

        ImGui::DragFloat("Radius##SSAOradius", &data.radius, 0.05f, 0.0f, 1.0f);
        ImGui::DragFloat("Bias##SSAObias", &data.bias, 0.0001f, 0.0f, 1.0f, "%.8f");
        ImGui::DragFloat("Power##SSAOpower", &data.power, 0.05f, 1.0f, 10.0f);

        ImGui::Text("Noise texture");
        noiseTexture->RenderImGui();

        ImGui::Text("Output texture");
        ImGui::Image(outputSRV, ImVec2(100, 100));

        blur.RenderImGui();
#endif
    }

}
