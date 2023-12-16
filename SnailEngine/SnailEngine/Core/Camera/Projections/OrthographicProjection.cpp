#include "stdafx.h"
#include "OrthographicProjection.h"

namespace Snail
{
OrthographicProjection::OrthographicProjection()
{
    nearPlane = DEFAULT_NEAR_PLANE;
    farPlane = DEFAULT_FAR_PLANE;
}

Matrix OrthographicProjection::SetupProjectionMatrix(const long screenWidth, const long screenHeight)
{
    width = screenWidth;
    height = screenHeight;
    return GetProjectionMatrix();
}

Matrix OrthographicProjection::GetProjectionMatrix()
{
    // Inversion of the z-planes is on purpose
    return Matrix::CreateOrthographic(static_cast<float>(width) / zoom, static_cast<float>(height) / zoom, farPlane, nearPlane);
}

Matrix OrthographicProjection::GetProjectionMatrix(const float nearPl, const float farPl)
{
    // Inversion of the z-planes is on purpose
    return Matrix::CreateOrthographic(static_cast<float>(width) / zoom, static_cast<float>(height) / zoom, farPl, nearPl);
}

#ifdef _IMGUI_
bool OrthographicProjection::RenderImGui(Camera* cameraPerspective)
{
    if (Projection::RenderImGui(cameraPerspective))
    {
        ImGui::Text("Zoom: ");
        ImGui::SameLine();
        ImGui::DragFloat("##camera zoom drag float", &zoom, 1, 0, 1000);
        return true;
    }
    return false;
}
#else
bool OrthographicProjection::RenderImGui(Camera*)
{
    return false;
}
#endif
}
