#pragma once
#include "Projection.h"

namespace Snail
{
struct PerspectiveProjection : Projection
{
    static constexpr float DEFAULT_FOV = DirectX::XM_PI / 4.0f;
    float fov = DEFAULT_FOV;

    Matrix SetupProjectionMatrix(long screenWidth, long screenHeight) override;
    Matrix GetProjectionMatrix() override;
    Matrix GetProjectionMatrix(float nearPl, float farPl) override;
    bool RenderImGui(Camera* cameraPerspective) override;
};
}
