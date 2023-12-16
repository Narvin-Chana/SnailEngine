#pragma once
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/D3D11Device.h"
#include "Rendering/TexturedMaterial.h"

namespace Snail
{
class EffectsShader;

class SubMesh
{
public:
    uint32_t indexBufferCount = 0;
    uint32_t indexBufferStartIndex = 0;

    SubMesh() = default;
    SubMesh(const SubMesh& subMesh);
    SubMesh& operator=(const SubMesh& subMesh);
    virtual ~SubMesh() = default;

    virtual void DrawGeometry(D3D11Device* renderDevice, int instancesCount = 1);

    const D3D11Buffer& GetMaterialBuffer() const noexcept;
    TexturedMaterial GetMaterial() const noexcept { return material; }
    void SetMaterial(const TexturedMaterial& mat);

    void RenderImGui(int id);
private:
    TexturedMaterial material;
    mutable D3D11Buffer materialBuffer{D3D11Buffer::CreateConstantBuffer<Material>()};

    mutable bool isBufferDirty = true;
};

}
