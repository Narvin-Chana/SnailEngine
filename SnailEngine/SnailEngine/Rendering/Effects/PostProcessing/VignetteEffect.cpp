#include "stdafx.h"
#include "VignetteEffect.h"

#include "Rendering/D3D11Device.h"

namespace Snail
{

VignetteEffect::VignetteEffect()
    : PostProcessEffect(L"SnailEngine/Shaders/PostProcessing/Vignette.cs.hlsl")
{}
void VignetteEffect::RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output)
{
    UNREFERENCED_PARAMETER(renderDevice);
    if (isActive)
    {
        computeShader->BindComputedUAV(output);
        computeShader->SetConstantBuffer("VignetteParameters", dataBuffer.GetBuffer());
        computeShader->Bind();
        // Get the dimensions of the UAV texture
        D3D11_TEXTURE2D_DESC desc;
        tex->GetDesc(&desc);
        const UINT width = desc.Width, height = desc.Height;
        // Determine the number of thread groups based on the dimensions
        const int numGroupsX = (width + 31) / 32;
        const int numGroupsY = (height + 31) / 32;
        computeShader->Execute(numGroupsX, numGroupsY, 1);
        computeShader->Unbind(); // Could put this in execute...
    }
}

void VignetteEffect::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::SeparatorText("Vignette");

    ImGui::Checkbox("Active##vignette", &isActive);

    VignetteParameters newParameters = data;

    ImGui::Text("Color (rgba): ");
    ImGui::SameLine();
    ImGui::DragFloat4("##Color vignette", &newParameters.color.x, 0.01f, 0, 1);

    ImGui::Text("RadiusDivisionFactor: ");
    ImGui::SameLine();
    ImGui::DragFloat("##RadiusDivisionFactor vignette", &newParameters.radiusDivisionFactor, 0.1f, 0, 100);

    ImGui::Text("Intensity: ");
    ImGui::SameLine();
    ImGui::DragFloat("##Intensity vignette", &newParameters.intensity, 0.01f, 0, 200);

    if (data.intensity != newParameters.intensity || data.radiusDivisionFactor != newParameters.radiusDivisionFactor || data.color != newParameters.color)
    {
        SetData(newParameters);
    }

#endif
}

}
