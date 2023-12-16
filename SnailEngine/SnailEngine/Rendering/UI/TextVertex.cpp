#include "stdafx.h"
#include "TextVertex.h"

namespace Snail
{
    D3D11_INPUT_ELEMENT_DESC TextVertex::layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    UINT TextVertex::elementCount = ARRAYSIZE(TextVertex::layout);
}
