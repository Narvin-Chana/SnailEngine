#include "stdafx.h"
#include "Effect.h"

namespace Snail
{
bool Effect::IsActive() const noexcept
{
    return isActive;
}

void Effect::SetActive(const bool isEffectActive)
{
    isActive = isEffectActive;
}
}
