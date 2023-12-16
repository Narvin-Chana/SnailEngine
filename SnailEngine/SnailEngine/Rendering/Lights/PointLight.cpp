#include "stdafx.h"
#include "PointLight.h"

namespace Snail
{
#ifdef _IMGUI_
void PointLight::RenderImGui(const int id)
{
    ImGui::Text("Color: ");
    ImGui::DragFloat3(("##pointlight color drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Color), 0.05f, 0, 1);

    ImGui::Text("Position: ");
    ImGui::DragFloat3(("##pointlight Position drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Position));

    ImGui::Text("Coefficients: ");
    ImGui::DragFloat3(("##pointlight Coefficients drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Coefficients), 0.05f, 0, 100);
}
#else
void PointLight::RenderImGui(const int) {}
#endif
}
