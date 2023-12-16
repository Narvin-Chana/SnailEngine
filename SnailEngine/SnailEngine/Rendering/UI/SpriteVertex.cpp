#include "stdafx.h"
#include "SpriteVertex.h"

namespace Snail
{
    // Definir l�organisation de notre sommet
    D3D11_INPUT_ELEMENT_DESC SpriteVertex::layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_INSTANCE_DATA, 0},
    };
    UINT SpriteVertex::elementCount = ARRAYSIZE(SpriteVertex::layout); 

    SpriteVertex::SpriteVertex(const Vector2& position, const Vector2& uv)
        : position{ position }
        , uv{ uv }
    {}
}
