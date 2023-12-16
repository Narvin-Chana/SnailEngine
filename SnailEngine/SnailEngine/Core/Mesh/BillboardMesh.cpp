#include "stdafx.h"
#include "BillboardMesh.h"

#include "Rendering/InputAssembler.h"

namespace Snail
{
void BillboardMesh::InitShaders()
{
    effectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/Billboard.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT);
}

void BillboardMesh::BindTextures(const SubMesh&) const
{
    assert(quadTexture);
    effectsShader->BindTexture("Billboard", quadTexture);
}

void BillboardMesh::Draw(const D3D11Buffer*)
{
    if (instancesModelMatrix.empty())
        return;

    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();

    instanceBuffer.UpdateData(instancesModelMatrix);

    // Must have an initialised shader
    assert(effectsShader.get());
    InputAssembler::SetPrimitiveTopology(primitiveTopology);
    InputAssembler::SetVertexBuffer(vertexBuffer, sizeof(MeshVertex), 0);
    InputAssembler::SetInstanceBuffer(instanceBuffer, sizeof(InstanceVertex), 0);
    InputAssembler::SetIndexBuffer(indexBuffer);

    switch (cullingType)
    {
    case CullingType::FRONT:
        device->SetFrontFaceCulling();
        break;
    case CullingType::NO:
        device->SetNoCulling();
        break;
    case CullingType::BACK:
    default:
        device->SetBackFaceCulling();
    }

    for (SubMesh& submesh : submeshes)
    {
        if (submesh.indexBufferCount == 0)
            continue;

        BindTextures(submesh);
        BindShaders();

        submesh.DrawGeometry(device, static_cast<int>(instancesModelMatrix.size()));
    }

    instancesModelMatrix.clear();
}

void BillboardMesh::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text("Quad Texture:");
    quadTexture->RenderImGui();
#endif
}
}
