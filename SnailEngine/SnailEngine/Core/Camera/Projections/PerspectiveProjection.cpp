#include "stdafx.h"
#include "PerspectiveProjection.h"

#include "Core/Scene.h"
#include "Core/WindowsEngine.h"
#include "Core/Camera/Camera.h"
#include "Entities/Entity.h"

namespace Snail
{

Matrix PerspectiveProjection::SetupProjectionMatrix(const long screenWidth, const long screenHeight)
{
    // TODO: unset dirty flag
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    return GetProjectionMatrix();
}

Matrix PerspectiveProjection::GetProjectionMatrix()
{
    // Invert far and nearplanes for inverse depth
    return Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, farPlane, nearPlane);
}

Matrix PerspectiveProjection::GetProjectionMatrix(const float nearPl, const float farPl)
{
    // Don't inverse far and nearplanes here
    return Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, std::max(nearPl, nearPlane), std::min(farPl, farPlane));
}

#ifdef _IMGUI_
bool PerspectiveProjection::RenderImGui(Camera* cameraPerspective)
{
    if (Projection::RenderImGui(cameraPerspective))
    {
        ImGui::Text("Near Plane: %f", nearPlane);
        ImGui::Text("Far Plane: %f", farPlane);

        const auto fovPrev = fov;

        ImGui::Text("FOV: ");
        ImGui::SameLine();
        ImGui::SliderAngle("##camera fov drag float", &fov, 30, 160);

        if (fovPrev != fov)
        {
            // TODO: just set dirty flag
            cameraPerspective->projectionMatrix = GetProjectionMatrix();
        }

        if (cameraPerspective->targetEntity)
        {
            ImGui::Text(("Current target: " + cameraPerspective->targetEntity->entityName).c_str());
            if (ImGui::Button("Unset parent"))
            {
                cameraPerspective->SetCameraTarget(Camera::CameraFollowMode::CAM_FREECAM);
            }
        }
        else
        {
            ImGui::Text("Current target: No target.");
            if (ImGui::Button("Select a target entity"))
            {
                ImGui::OpenPopup("Target Entity Selector");
            }
            if (ImGui::BeginPopupModal("Target Entity Selector", nullptr))
            {
                ImGui::Text("Select an entity to target: ");

                static bool recalculateSceneNames = true;
                static std::vector<std::string> sceneObjNames;
                static int itemCurrentIndex = 0;

                const Scene* scene = WindowsEngine::GetScene();

                if (recalculateSceneNames)
                {
                    auto sceneObjs = scene->GetEntities();
                    sceneObjNames.clear();
                    std::ranges::transform(sceneObjs,
                        std::back_inserter(sceneObjNames),
                        [](const Entity* entity) { return entity->entityName; });
                    itemCurrentIndex = 0;
                    recalculateSceneNames = false;
                }

                if (ImGui::BeginCombo("##cb", !sceneObjNames.empty() ? sceneObjNames[itemCurrentIndex].c_str() : "No scene entities..."))
                {
                    for (int n = 0; n < sceneObjNames.size(); n++)
                    {
                        const bool isSelected = (itemCurrentIndex == n);
                        if (ImGui::Selectable(sceneObjNames[n].c_str(), isSelected))
                            itemCurrentIndex = n;
                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                static Camera::CameraFollowMode modeSelected = Camera::CAM_FREECAM;
                ImGui::Text("Target Mode: ");

                if (ImGui::RadioButton("First Person##FPP", modeSelected == Camera::CAM_FIRST_PERSON))
                {
                    modeSelected = Camera::CAM_FIRST_PERSON;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Third Person##TPP", modeSelected == Camera::CAM_THIRD_PERSON))
                {
                    modeSelected = Camera::CAM_THIRD_PERSON;
                }

                if (modeSelected != Camera::CAM_FREECAM)
                {
                    if (ImGui::Button("Confirm"))
                    {
                        cameraPerspective->SetCameraTarget(modeSelected, scene->GetEntities()[itemCurrentIndex]);
                        recalculateSceneNames = true;
                        modeSelected = Camera::CAM_FREECAM;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                }
                if (ImGui::Button("Close"))
                {
                    recalculateSceneNames = true;
                    modeSelected = Camera::CAM_FREECAM;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

    }
    return true;
}
#else
bool PerspectiveProjection::RenderImGui(Camera*)
{
    return false;
}
#endif

}
