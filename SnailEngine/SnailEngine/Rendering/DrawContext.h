#pragma once

namespace Snail
{
class RendererModule;
class D3D11Device;

class DrawContext
{
protected:
    template <class T>
    void DrawBoundingBox(const T& bb, bool hit);

public:
    RendererModule* renderer;
    D3D11Device* device;

    DrawContext(RendererModule* renderer, D3D11Device* device);
    virtual ~DrawContext() = default;

    virtual bool ShouldBeCulled(const DirectX::BoundingBox&);
    virtual bool ShouldBeCulled(const DirectX::BoundingOrientedBox&);
    virtual bool ShouldBeCulled(const DirectX::BoundingSphere&);
};

}
