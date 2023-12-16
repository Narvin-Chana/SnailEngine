#pragma once
#include "CubeMesh.h"

namespace Snail
{

class DecalMesh : public CubeMesh
{
public:
    DecalMesh()
        : matricesBuffer(D3D11Buffer::CreateConstantBuffer<MatricesBuffer>()) {}

protected:
    struct MatricesBuffer
    {
        Matrix invViewMatrix;
        Matrix invProjMatrix;
    };

    D3D11Buffer matricesBuffer;

public:
    void Draw(const D3D11Buffer* viewProjBuffer) override;

protected:
    void InitShaders() override;
    void BindTextures(const SubMesh& submesh) const override;
    void BindBuffers(const D3D11Buffer* vsMatrixes) override;
};

}
