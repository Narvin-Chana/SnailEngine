#include "stdafx.h"

#include "Mesh.h"

#include <algorithm>

#include "Core/WindowsEngine.h"
#include "Rendering/InputAssembler.h"

using namespace DirectX::SimpleMath;

namespace Snail
{
BaseMesh::BaseMesh(const D3D11_PRIMITIVE_TOPOLOGY topology)
    : primitiveTopology(topology)
{ }

void BaseMesh::SetCullingType(const CullingType culling)
{
    cullingType = culling;
}

void BaseMesh::SetTranslucent(const bool newValue)
{
    const bool isTranslucent = effectsShader->HasDefine(THIN_TRANSLUCENCY_DEFINE);
    if (isTranslucent == newValue)
        return;

    std::lock_guard lock{ DeviceMutex };

    if (newValue)
        effectsShader->AddDefine(THIN_TRANSLUCENCY_DEFINE);
    else
        effectsShader->RemoveDefine(THIN_TRANSLUCENCY_DEFINE);
}

void BaseMesh::ReloadShader()
{
    effectsShader->ReloadShader();
}

void BaseMesh::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text(("Name: " + name).c_str());
#endif
}

template class Mesh<uint16_t>;
template class Mesh<uint32_t>;

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::BindBuffers(const D3D11Buffer* viewProjMatrixes)
{
    if (viewProjMatrixes)
    {
        effectsShader->SetConstantBuffer("TransformMatrixes", viewProjMatrixes->GetBuffer());
    }
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::Init()
{
    usesBlending |= std::ranges::any_of(submeshes,
        [](const auto& sm)
        {
            return sm.GetMaterial().isBlending;
        });

    InitShaders();
    InitBuffers();
}

template <class IdxType> requires std::is_integral_v<IdxType>
Mesh<IdxType>::Mesh(const D3D11_PRIMITIVE_TOPOLOGY topology)
    : BaseMesh(topology)
    , renderDevice{WindowsEngine::GetInstance().GetRenderDevice()}
    , usesBlending(false)
    , vertexBuffer{D3D11_BIND_VERTEX_BUFFER}
    , indexBuffer{D3D11_BIND_INDEX_BUFFER}
    , instanceBuffer{D3D11_BIND_VERTEX_BUFFER}
{ }

template <class IdxType> requires std::is_integral_v<IdxType>
Mesh<IdxType>::~Mesh() {}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::SetEnableBlending(const bool newValue)
{
    if (newValue == usesBlending)
        return;

    usesBlending = newValue;
    InitShaders();
}

template <class IdxType> requires std::is_integral_v<IdxType>
bool Mesh<IdxType>::GetBlendingEnabled() const
{
    return usesBlending;
}

template <class IdxType> requires std::is_integral_v<IdxType>
std::vector<IdxType>& Mesh<IdxType>::GetIndexes() { return indexes; }

template <class IdxType> requires std::is_integral_v<IdxType>
std::vector<MeshVertex>& Mesh<IdxType>::GetVertices() { return vertices; }

template <class IdxType> requires std::is_integral_v<IdxType>
uint32_t Mesh<IdxType>::GetIndexCount() { return static_cast<uint32_t>(indexes.size()); }

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::SubscribeInstance(const Matrix m)
{
    instancesModelMatrix.emplace_back(m.Transpose(), m.Invert().Transpose());
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::BindTextures(const SubMesh& submesh) const
{
    static TextureManager& textureManager = WindowsEngine::GetModule<TextureManager>();
    const TexturedMaterial material = submesh.GetMaterial();

    effectsShader->BindTexture("NormalMap", textureManager.GetAsset<Texture2D>(material.normalMapTexture));
    effectsShader->BindTexture("Diffuse", textureManager.GetAsset<Texture2D>(material.diffuseTexture));

    if (usesBlending)
    {
        effectsShader->BindTexture("PrimaryBlendDiffuse", textureManager.GetAsset<Texture2D>(material.primaryBlendDiffuseTexture));
        effectsShader->BindTexture("PrimaryBlend", textureManager.GetAsset<Texture2D>(material.primaryBlendTexture));
        effectsShader->BindTexture("SecondaryBlendDiffuse", textureManager.GetAsset<Texture2D>(material.secondaryBlendDiffuseTexture));
        effectsShader->BindTexture("SecondaryBlend", textureManager.GetAsset<Texture2D>(material.secondaryBlendTexture));
    }

    effectsShader->SetConstantBuffer("MaterialParameters", submesh.GetMaterialBuffer().GetBuffer());
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::InitShaders()
{
    std::unordered_set<std::string> defines;
    if (usesBlending)
        defines.insert(TEXTURE_BLENDING_DEFINE);

    effectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/DeferredPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT, defines);
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::InitBuffers()
{
    CalculateTangents();

    vertexBuffer = D3D11Buffer(D3D11_BIND_VERTEX_BUFFER, vertices);

    indexBuffer = D3D11Buffer(D3D11_BIND_INDEX_BUFFER, indexes);
    indexBuffer.SetBufferElementWidth(sizeof(IdxType));
}
template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::BindShaders()
{
    effectsShader->Bind();
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::CalculateTangents()
{
    if (primitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
        return;

    for (int i = 0; i < indexes.size(); i += 3)
    {
        auto i0 = indexes[i];
        auto i1 = indexes[i + 1];
        auto i2 = indexes[i + 2];

        auto& v0 = vertices[i0];
        auto& v1 = vertices[i1];
        auto& v2 = vertices[i2];
        Vector3 p0 = v0.position;
        Vector3 p1 = v1.position;
        Vector3 p2 = v2.position;
        Vector2 w0 = v0.uv;
        Vector2 w1 = v1.uv;
        Vector2 w2 = v2.uv;

        Vector3 e1 = p1 - p0, e2 = p2 - p0;
        Vector2 uv1 = w1 - w0, uv2 = w2 - w0;

        // These conditions are here to avoid having a division by zero
        // by altering a little bit the x and y values
        if (uv1.x * uv2.y == 0)
        {
            uv1.x -= std::numeric_limits<float>::epsilon();
            uv2.y += std::numeric_limits<float>::epsilon();
        }

        if (uv2.x * uv1.y == 0)
        {
            uv1.y += std::numeric_limits<float>::epsilon();
            uv2.x += std::numeric_limits<float>::epsilon();
        }

        if (uv1.x * uv2.y - uv2.x * uv1.y == 0)
        {
            uv1.x += std::numeric_limits<float>::epsilon();
        }

        const float r = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);
        Vector3 t = (e1 * uv2.y - e2 * uv1.y) * r;
        Vector3 b = (e2 * uv1.x - e1 * uv2.x) * r;

        v0.bitangent += b;
        v1.bitangent += b;
        v2.bitangent += b;
        v0.tangent += t;
        v1.tangent += t;
        v2.tangent += t;
    }

    constexpr auto reject = [](const Vector3& a, const Vector3& b)
    {
        return a - a.Dot(b) / b.Dot(b) * b;
    };

    for (auto& v : vertices)
    {
        v.tangent = reject(v.tangent, v.normal);
        v.tangent.Normalize();
    }
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::CalculateBounds()
{
    // Reset bounds
    minBounds = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    maxBounds = Vector3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (const MeshVertex& vertex : vertices)
    {
        minBounds.x = std::min(minBounds.x, vertex.position.x);
        maxBounds.x = std::max(maxBounds.x, vertex.position.x);

        minBounds.y = std::min(minBounds.y, vertex.position.y);
        maxBounds.y = std::max(maxBounds.y, vertex.position.y);

        minBounds.z = std::min(minBounds.z, vertex.position.z);
        maxBounds.z = std::max(maxBounds.z, vertex.position.z);
    }

    boundsAreDirty = false;
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::Draw(const D3D11Buffer* viewProjBuffer)
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

        BindBuffers(viewProjBuffer);
        BindTextures(submesh);
        BindShaders();

        submesh.DrawGeometry(renderDevice, static_cast<int>(instancesModelMatrix.size()));
    }

    instancesModelMatrix.clear();
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::DrawGeometry()
{
    if (instancesModelMatrix.empty())
        return;

    instanceBuffer.UpdateData(instancesModelMatrix);

    InputAssembler::SetPrimitiveTopology(primitiveTopology);
    InputAssembler::SetVertexBuffer(vertexBuffer, sizeof(MeshVertex), 0);
    InputAssembler::SetInstanceBuffer(instanceBuffer, sizeof(InstanceVertex), 0);
    InputAssembler::SetIndexBuffer(indexBuffer);

    for (SubMesh& submesh : submeshes)
    {
        if (submesh.indexBufferCount == 0)
            continue;

        submesh.DrawGeometry(renderDevice, static_cast<int>(instancesModelMatrix.size()));
    }

    instancesModelMatrix.clear();
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::SetAllMaterialMember(const std::string& textureFilepath, std::string TexturedMaterial::* materialMember)
{
    std::ranges::for_each(submeshes,
        [&](SubMesh& subMesh)
        {
            TexturedMaterial mat = subMesh.GetMaterial();
            mat.*materialMember = textureFilepath;
            subMesh.SetMaterial(mat);
        });
}

template <class IdxType> requires std::is_integral_v<IdxType>
void Mesh<IdxType>::RenderImGui()
{
#ifdef _IMGUI_
    BaseMesh::RenderImGui();
    ImGui::Text("Vertex Count: %d", vertices.size());
    ImGui::Text("Index Count: %d", indexes.size());
    ImGui::Text("Submesh Count: %d", submeshes.size());

    if (ImGui::TreeNode(("Submeshes: ##" + name).c_str()))
    {
        for (int i = 0; i < submeshes.size(); ++i)
        {
            if (ImGui::TreeNode(("Submesh " + std::to_string(i) + "##submesh" + std::to_string(i)).c_str()))
            {
                submeshes[i].RenderImGui(i);
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

#endif
}

}
