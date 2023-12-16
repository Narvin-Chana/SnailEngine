#pragma once
#include "Util/Util.h"

namespace Snail
{
struct DirectionalLight
{
    DX_ALIGN Vector3 Color{1, 1, 1};
    DX_ALIGN_BOOL bool castsShadows = true;
    DX_ALIGN Vector3 Direction{0, -1, 0};
    DX_ALIGN_BOOL bool isActive = true;

    void RenderImGui(int id);
};
};
