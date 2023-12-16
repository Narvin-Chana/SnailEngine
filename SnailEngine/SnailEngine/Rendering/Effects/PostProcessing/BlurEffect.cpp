#include "stdafx.h"
#include "BlurEffect.h"

#include <random>

#include "Core/WindowsEngine.h"
#include "Rendering/D3D11Device.h"

namespace Snail
{

    BlurEffect::BlurEffect()
        : PostProcessEffect(L"SnailEngine/Shaders/PostProcessing/Blur.cs.hlsl")
    {
    }

    void BlurEffect::RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output)
    {
        UNREFERENCED_PARAMETER(renderDevice);
        if (isActive)
        {
            computeShader->BindComputedUAV(output);

            SetData(data);
            computeShader->SetConstantBuffer("BlurParams", dataBuffer.GetBuffer());

            computeShader->Bind();
            // Get the dimensions of the UAV texture
            D3D11_TEXTURE2D_DESC desc;
            tex->GetDesc(&desc);
            const UINT width = desc.Width, height = desc.Height;
            // Determine the number of thread groups based on the dimensions
            const int numGroupsX = (width + 7) / 8;
            const int numGroupsY = (height + 7) / 8;
            computeShader->Execute(numGroupsX, numGroupsY, 1);
            computeShader->Unbind(); // Could put this in execute...
        }
    }

    void BlurEffect::RenderImGui()
    {
#ifdef _IMGUI_
        ImGui::SeparatorText("Blur");

        ImGui::Checkbox("Active##Blur", &isActive);

        ImGui::DragInt("Kernel Size##BlurKernelSize", &data.kernelSize, 1, 0, 100);
#endif
    }

}
