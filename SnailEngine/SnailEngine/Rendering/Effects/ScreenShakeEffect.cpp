#include "stdafx.h"
#include "ScreenShakeEffect.h"

#include "Core/WindowsEngine.h"
#include "Core/Camera/Camera.h"

namespace Snail
{

void ScreenShakeEffect::Update(const float dt)
{
    if (isActive)
    {
        params.time -= dt;

        if (params.time <= 0)
        {
            isActive = false;
            params = {};
        }
    }
}

void ScreenShakeEffect::RenderEffectBegin()
{
    Camera* cam = WindowsEngine::GetCamera();
    assert(cam);
    Transform cameraTransform = cam->GetTransform();
    prevPos = cameraTransform.position;

    const Vector3 rightVector = cam->GetTransform().GetRightVector();
    const Vector3 upVector = cam->GetTransform().GetUpVector();
    const Vector3 newPos = cam->GetTransform().position + 
        rightVector * sin(params.time / params.period * 2 * DirectX::XM_PI) * params.intensity.x +
        upVector * sin(params.time / params.period * 2 * DirectX::XM_PI) * params.intensity.y;

    cameraTransform.position = newPos;

    cam->SetTransform(cameraTransform);
}

void ScreenShakeEffect::RenderEffectEnd() const
{
    Camera* cam = WindowsEngine::GetCamera();
    assert(cam);
    Transform cameraTransform = cam->GetTransform();
    cameraTransform.position = prevPos;

    cam->SetTransform(cameraTransform);
}

void ScreenShakeEffect::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::SeparatorText("Screen Shake");

    ImGui::Checkbox("Active##screenshake", &isActive);

    ImGui::Text("Intensity (x ,y): ");
    ImGui::SameLine();
    ImGui::DragFloat2("##Intensity screenshake", &params.intensity.x, 0.1f);

    ImGui::Text("Time: ");
    ImGui::SameLine();
    ImGui::DragFloat("##Time screenshake", &params.time, 0.1f);

    ImGui::Text("Period: ");
    ImGui::SameLine();
    ImGui::DragFloat("##Period screenshake", &params.period, 0.1f);
#endif
}

}
