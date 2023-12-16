#include "stdafx.h"
#include "AdaptiveLightingTrigger.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

AdaptiveLightingTrigger::AdaptiveLightingTrigger(const Params& params)
    : TriggerBox{params}
{
    static auto* scene = WindowsEngine::GetScene();
    scene->SetVolumetricFactor(lowerBound);
}

void AdaptiveLightingTrigger::Update(float dt) noexcept
{
    TriggerBox::Update(dt);

    static auto* scene = WindowsEngine::GetScene();

    const float val = scene->GetVolumetricFactor();
    if (shouldIncrement)
        scene->SetVolumetricFactor(std::min(val + incrementValue * dt, upperBound));
    else
        scene->SetVolumetricFactor(std::max(val - incrementValue * dt, lowerBound));
}
void AdaptiveLightingTrigger::OnTriggerEnter()
{
    TriggerBox::OnTriggerEnter();
    shouldIncrement = true;
}
void AdaptiveLightingTrigger::OnTriggerExit()
{
    TriggerBox::OnTriggerExit();
    shouldIncrement = false;
}
void AdaptiveLightingTrigger::RenderImGui(int idNumber)
{
    TriggerBox::RenderImGui(idNumber);

#ifdef _IMGUI_
    ImGui::DragFloat("Upper bound##upperbound", &upperBound, 0.05f, 0, 1);
    ImGui::DragFloat("Lower bound##lowerbound", &lowerBound, 0.05f, 0, 1);
    ImGui::DragFloat("Increment step##incstep", &incrementValue, 0.05f, 0, 1);
#endif
}

}
