#pragma once

#include <d3d11.h>

#include "Core/Math/SimpleMath.h"

namespace Snail
{
    struct SpriteVertex
    {
        static UINT elementCount;
        static D3D11_INPUT_ELEMENT_DESC layout[];

        SpriteVertex() = default;
        SpriteVertex(
            const Vector2& position,
            const Vector2& uv = {}
        );

        Vector2 position{};
        Vector2 uv{};
    };
} // namespace Snail
