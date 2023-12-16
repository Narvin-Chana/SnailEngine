#include "stdafx.h"
#include "Decal.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
Decal::Params::Params()
{
    name = "Decal";
    // Decal without mesh makes no sense!
    mesh = nullptr;
}

Decal::Decal(const Params& params)
    : Entity(params)
{
    castsShadows = false;
}

std::string Decal::GetJsonType()
{
    return "decal";
}
}
