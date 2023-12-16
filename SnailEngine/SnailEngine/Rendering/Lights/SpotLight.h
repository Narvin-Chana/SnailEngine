#pragma once
#include "Util/Util.h"

namespace Snail
{
struct SpotLight
{
    DX_ALIGN Vector3 Position{0, 0, 0};
    DX_ALIGN Vector3 Color{1, 1, 1};
    DX_ALIGN Vector3 Direction{0, -1, 0};
    DX_ALIGN Vector3 Coefficients{1, 0.045f, 0.0075f};
    // Values for a distance of 100 (https://learnopengl.com/Lighting/Light-casters)
    float InnerConeAngle{30.0f};
    float OuterConeAngle{45.0f};
    DX_ALIGN_BOOL bool isActive = true;

    void RenderImGui(int id);
};
};
