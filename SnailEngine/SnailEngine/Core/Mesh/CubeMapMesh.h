#pragma once

#include "Mesh.h"

namespace Snail
{

class CubeMapMesh final : public Mesh<uint16_t>
{
public:
    TextureCube* cubeMapTexture;
    mutable D3D11Buffer data;

    CubeMapMesh();

protected:
    mutable D3D11Buffer volumetricDensityFactor;

    void InitShaders() override;

    void BindBuffers(const D3D11Buffer* vsMatrixes) override;
    void BindTextures(const SubMesh& submesh) const override;
};

}
