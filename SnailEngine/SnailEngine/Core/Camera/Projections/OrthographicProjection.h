#pragma once
#include "Projection.h"

namespace Snail
{

struct OrthographicProjection : Projection
{
    static constexpr float DEFAULT_NEAR_PLANE = -200.0f;
    static constexpr float DEFAULT_FAR_PLANE = 200.0f;

    float zoom = 2.0f;
    long width = 0, height = 0;

    OrthographicProjection();
    Matrix SetupProjectionMatrix(long screenWidth, long screenHeight) override;
    Matrix GetProjectionMatrix() override;
    Matrix GetProjectionMatrix(float nearPl, float farPl) override;
    bool RenderImGui(Camera* cameraPerspective) override;
};

}
