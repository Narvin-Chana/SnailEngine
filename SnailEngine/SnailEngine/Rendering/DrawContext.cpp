#include "stdafx.h"
#include "DrawContext.h"

#include "Core/RendererModule.h"

namespace Snail
{
    DrawContext::DrawContext(RendererModule* renderer, D3D11Device* device)
        : renderer{ renderer }
        , device{ device }
    {}

    bool DrawContext::ShouldBeCulled(const DirectX::BoundingBox&) {
        return false;
    }
    bool DrawContext::ShouldBeCulled(const DirectX::BoundingOrientedBox&) {
        return false;
    }
    bool DrawContext::ShouldBeCulled(const DirectX::BoundingSphere&) {
        return false;
    }

    template void DrawContext::DrawBoundingBox(const DirectX::BoundingOrientedBox& obb, bool hit);
    template void DrawContext::DrawBoundingBox(const DirectX::BoundingBox& bb, bool hit);

    template <class T>
    void DrawContext::DrawBoundingBox(const T& bb, const bool hit)
    {
        renderer->DrawBoundingBox(bb, hit ? Color{ 1, 0, 0 } : Color{ 0, 1, 0 });
    }
}
