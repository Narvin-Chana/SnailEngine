#include "stdafx.h"
#include "ChromaticAberrationEffect.h"

#include "Rendering/D3D11Device.h"

namespace Snail
{

ChromaticAberrationEffect::ChromaticAberrationEffect()
    : PostProcessEffect(L"SnailEngine/Shaders/PostProcessing/ChromaticAberration.cs.hlsl")
{}

void ChromaticAberrationEffect::RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output)
{
    UNREFERENCED_PARAMETER(renderDevice);
    if (isActive)
    {
        computeShader->BindComputedUAV(output);
        computeShader->SetConstantBuffer("ChromaticAberrationParameters", dataBuffer.GetBuffer());
        computeShader->Bind();
        // Get the dimensions of the UAV texture
        D3D11_TEXTURE2D_DESC desc;
        tex->GetDesc(&desc);
        const UINT width = desc.Width, height = desc.Height;
        // Determine the number of thread groups based on the dimensions
        const int numGroupsX = (width + 15) / 16;
        const int numGroupsY = (height + 15) / 16;
        computeShader->Execute(numGroupsX, numGroupsY, 1);
        computeShader->Unbind(); // Could put this in execute...
    }
}

void ChromaticAberrationEffect::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::SeparatorText("Chromatic Aberration");

    ImGui::Checkbox("Active##chromatic aberration", &isActive);

    ChromaticAberrationParameters newParameters = data;

    ImGui::Text("RGB Offsets: ");
    ImGui::SameLine();
    ImGui::DragFloat3("##RGB chromatic aberration", &newParameters.rgbOffset.x, 0.01f);
    
    ImGui::Text("Intensity: ");
    ImGui::SameLine();
    ImGui::DragFloat("##Intensity chromatic aberration", &newParameters.intensity, 0.25f);

    if (data.rgbOffset != newParameters.rgbOffset || data.intensity != newParameters.intensity)
    {
        SetData(newParameters);
    }
#endif
}

}
