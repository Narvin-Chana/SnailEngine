#pragma once

#include <d3d11.h>

namespace Snail
{
    struct TextVertex
    {
        static UINT elementCount;
        static D3D11_INPUT_ELEMENT_DESC layout[];
    };
} // namespace Snail
