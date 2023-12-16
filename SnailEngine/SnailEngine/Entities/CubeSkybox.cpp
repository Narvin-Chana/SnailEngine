#include "stdafx.h"
#include "CubeSkybox.h"

#include "Core/WindowsEngine.h"
#include "Core/Mesh/CubeMapMesh.h"

namespace Snail
{
CubeSkybox::Params::Params()
{
    static MeshManager& meshManager = WindowsEngine::GetModule<MeshManager>();

    name = "Skybox";
    mesh = meshManager.GetAsset<CubeMapMesh>("CubeMap");
    castsShadows = false;
}

CubeSkybox::CubeSkybox(const Params& params)
    : Entity(params)
{
    shouldFrustumCull = false;
}

void CubeSkybox::Draw(DrawContext& ctx)
{
    Entity::Draw(ctx);
    mesh->Draw(&WindowsEngine::GetCamera()->GetTransformMatrixesBuffer());
}

}
