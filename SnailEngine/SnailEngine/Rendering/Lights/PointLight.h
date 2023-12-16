#pragma once
#include "Util/Util.h"

namespace Snail
{
struct PointLight
{
    DX_ALIGN Vector3 Position{0, 0, 0};
    DX_ALIGN Vector3 Color{1, 1, 1};
    DX_ALIGN Vector3 Coefficients{1, 0.045f, 0.0075f};
    DX_ALIGN_BOOL bool isActive = true;
    // Values for a distance of 100 (https://learnopengl.com/Lighting/Light-casters)

    void RenderImGui(int id);
};
};
