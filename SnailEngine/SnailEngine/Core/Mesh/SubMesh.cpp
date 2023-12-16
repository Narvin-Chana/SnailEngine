#include "stdafx.h"
#include "SubMesh.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

SubMesh::SubMesh(const SubMesh& subMesh)
    : material{subMesh.material}
{
    indexBufferCount = subMesh.indexBufferCount;
    indexBufferStartIndex = subMesh.indexBufferStartIndex;
}

SubMesh& SubMesh::operator=(const SubMesh& subMesh)
{
    indexBufferCount = subMesh.indexBufferCount;
    indexBufferStartIndex = subMesh.indexBufferStartIndex;
    material = subMesh.material;
    return *this;
}

void SubMesh::DrawGeometry(D3D11Device* renderDevice, const int instancesCount)
{
    renderDevice->DrawIndexedInstanced(indexBufferCount, instancesCount, indexBufferStartIndex);
}
const D3D11Buffer& SubMesh::GetMaterialBuffer() const noexcept
{
    if (isBufferDirty)
    {
        materialBuffer.UpdateData(material.material);
        isBufferDirty = false;
    }

    return materialBuffer;
}

void SubMesh::SetMaterial(const TexturedMaterial& mat)
{
    material = mat;
    isBufferDirty = true;
}

#ifdef _IMGUI_
void SubMesh::RenderImGui(const int id)
{
    
#ifdef _IMGUI_
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();
    ImGui::Text(("Index Start Index: " + std::to_string(indexBufferStartIndex)).c_str());
    ImGui::Text(("Index Count: " + std::to_string(indexBufferCount)).c_str());

    if (ImGui::TreeNode(("Material:##" + std::to_string(id)).c_str()))
    {
        if (ImGui::TreeNode(("Diffuse: " + material.diffuseTexture + "##" + material.diffuseTexture).c_str()))
        {
            tm.GetTexture2D(material.diffuseTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Primary Blend Diffuse Texture: " + material.primaryBlendDiffuseTexture + "##" + material.primaryBlendDiffuseTexture).c_str()))
        {
            tm.GetTexture2D(material.primaryBlendDiffuseTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Primary Blend Texture: " + material.primaryBlendTexture + "##" + material.primaryBlendTexture).c_str()))
        {
            tm.GetTexture2D(material.primaryBlendTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Secondary Blend Diffuse Texture: " + material.secondaryBlendDiffuseTexture + "##" + material.secondaryBlendDiffuseTexture).c_str()))
        {
            tm.GetTexture2D(material.secondaryBlendDiffuseTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Secondary Blend Texture: " + material.secondaryBlendTexture + "##" + material.secondaryBlendTexture).c_str()))
        {
            tm.GetTexture2D(material.secondaryBlendTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Normal Map: " + material.normalMapTexture + "##" + material.normalMapTexture).c_str()))
        {
            tm.GetTexture2D(material.normalMapTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Specular Texture: " + material.specularTexture + "##" + material.specularTexture).c_str()))
        {
            tm.GetTexture2D(material.specularTexture)->RenderImGui();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(("Ambient Texture: " + material.ambientTexture + "##" + material.ambientTexture).c_str()))
        {
            tm.GetTexture2D(material.ambientTexture)->RenderImGui();
            ImGui::TreePop();
        }

        ImGui::Text("Diffuse Factor:");
        ImGui::DragFloat3(("##Diffuse Factor" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.diffuse),
            0.05f,
            0.0f,
            1.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Specular Factor:");
        ImGui::DragFloat3(("##Specular Factor" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.specular),
            0.05f,
            0.0f,
            1.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Ambient Factor:");
        ImGui::DragFloat3(("##Ambient Factor" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.ambient),
            0.05f,
            0.0f,
            1.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Emission Factor:");
        ImGui::DragFloat3(("##Emission Factor" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.emission),
            0.05f,
            0.0f,
            1.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Specular Exponent (Shininess):");
        ImGui::DragFloat(("##Specular Exponent (Shininess)" + std::to_string(id)).c_str(),
            &material.material.shininess,
            1.0f,
            1.0f,
            256.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("UV Scale:");
        ImGui::DragFloat2(("##UV Scale" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.uvScale),
            0.05f,
            0.0f,
            1000.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Primary Blend UV Scale:");
        ImGui::DragFloat2(("##Primary Blend UV Scale" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.primaryBlendUvScale),
            0.05f,
            0.0f,
            1000.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Secondary Blend UV Scale:");
        ImGui::DragFloat2(("##Secondary Blend UV Scale" + std::to_string(id)).c_str(),
            reinterpret_cast<float*>(&material.material.secondaryBlendUvScale),
            0.05f,
            0.0f,
            1000.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp);

        if (ImGui::Button("Update Material"))
            isBufferDirty = true;

        ImGui::TreePop();

    }
#else
    UNREFERENCED_PARAMETER(id);
#endif
}
#else
void SubMesh::RenderImGui(const int)
{}
#endif
}
