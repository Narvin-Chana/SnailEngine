#include "stdafx.h"
#include "SpotLight.h"

namespace Snail
{
#ifdef _IMGUI_
void SpotLight::RenderImGui(const int id)
{
    ImGui::Text("Color: ");
    ImGui::DragFloat3(("##spotlight color drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Color), 0.05f, 0, 1);

    ImGui::Text("Direction: ");
    ImGui::DragFloat3(("##spotlight Direction drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Direction), 0.05f, -1, 1);
    Direction.Normalize();

    ImGui::Text("Position: ");
    ImGui::DragFloat3(("##spotlight Position drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Position));

    ImGui::Text("Coefficients: ");
    ImGui::DragFloat3(("##spotlight Coefficients drag float3" + std::to_string(id)).c_str(), reinterpret_cast<float*>(&Coefficients), 0.05f, 0, 100);
    
    ImGui::Text("Inner Cone Angle: ");
    ImGui::DragFloat(("##spotlight InnerConeAngle drag float" + std::to_string(id)).c_str(), &InnerConeAngle);

    ImGui::Text("Outer Cone Angle: ");
    ImGui::DragFloat(("##spotlight OuterConeAngle drag float" + std::to_string(id)).c_str(), &OuterConeAngle);
}
#else
void SpotLight::RenderImGui(const int)
{
}
#endif
}
