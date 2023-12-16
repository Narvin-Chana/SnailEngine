#include "stdafx.h"
#include "DecalMesh.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

void DecalMesh::Draw(const D3D11Buffer* viewProjBuffer)
{
    MatricesBuffer mb;
    const Camera* cam = WindowsEngine::GetCamera();

    mb.invViewMatrix = cam->GetViewMatrix().Invert().Transpose();
    mb.invProjMatrix = cam->GetProjectionMatrix().Invert().Transpose();
    matricesBuffer.UpdateData(mb);

    CubeMesh::Draw(viewProjBuffer);
    // Unbind depth after draw call
    effectsShader->UnbindResource("DepthTexture");
}

void DecalMesh::InitShaders()
{
    std::unordered_set<std::string> defines;
    if (usesBlending)
        defines.insert(TEXTURE_BLENDING_DEFINE);

    effectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/Decal.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT, defines);
}

void DecalMesh::BindTextures(const SubMesh& submesh) const
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    static TextureManager& textureManager = WindowsEngine::GetModule<TextureManager>();
    const TexturedMaterial material = submesh.GetMaterial();

    effectsShader->BindTexture("Diffuse", textureManager.GetAsset<Texture2D>(material.diffuseTexture));
    effectsShader->BindShaderResourceView("DepthTexture", device->GetDepthShaderResourceView());
}

void DecalMesh::BindBuffers(const D3D11Buffer* vsMatrixes)
{
    // Bind invViewProjMat
    effectsShader->SetConstantBuffer("InvViewMatrixes", matricesBuffer.GetBuffer());
    CubeMesh::BindBuffers(vsMatrixes);
}

}
