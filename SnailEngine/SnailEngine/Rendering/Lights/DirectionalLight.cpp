#include "stdafx.h"
#include "DirectionalLight.h"

#include "Util/DebugUtil.h"

namespace Snail
{
#ifdef _IMGUI_
void DirectionalLight::RenderImGui(const int id)
{
    ImGui::Text("Is Active: ");
    ImGui::Checkbox(("##dirlight isActive checkbox" + std::to_string(id)).c_str(), &isActive);

    ImGui::Text("Casts Shadows: ");
    ImGui::Checkbox(("##dirlight castsShadows checkbox" + std::to_string(id)).c_str(), &castsShadows);

    ImGui::Text("Color: ");
    ImGui::DragFloat3(("##dirlight color drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Color), 0.05f, 0.0f, 1.0f);

    ImGui::Text("Direction: ");
    ImGui::DragFloat3(("##dirlight direction drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Direction), 0.05f, -1.0f, 1.0f);
    Direction.Normalize();
}
#else
void DirectionalLight::RenderImGui(const int) {}
#endif
}
