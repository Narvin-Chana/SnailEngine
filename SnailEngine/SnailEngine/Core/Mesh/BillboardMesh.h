#pragma once
#include "QuadMesh.h"

namespace Snail
{

    class BillboardMesh : public QuadMesh
    {
    public:
        Texture2D* quadTexture = nullptr;
    protected:

        void InitShaders() override;
        void BindTextures(const SubMesh& submesh) const override;
        void Draw(const D3D11Buffer* viewProjBuffer) override;
    public:
        void RenderImGui() override;
    };

}
