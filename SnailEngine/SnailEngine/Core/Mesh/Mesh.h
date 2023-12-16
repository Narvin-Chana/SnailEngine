#pragma once

#include <PxPhysicsAPI.h>
#include <vector>

#include "SubMesh.h"
#include "Rendering/TexturedMaterial.h"
#include "Rendering/MeshVertex.h"
#include "Rendering/Shaders/EffectsShader.h"

namespace Snail
{
class D3D11Device;

class BaseMesh
{
protected:
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
    bool boundsAreDirty = true;

    virtual void InitBuffers() = 0;
    virtual void BindBuffers(const D3D11Buffer* vsMatrixes) = 0;
    virtual void BindTextures(const SubMesh& submesh) const = 0;
    virtual void BindShaders() = 0;

    static constexpr const char* THIN_TRANSLUCENCY_DEFINE = "THIN_TRANSLUCENCY";
public:
    enum class CullingType
    {
        FRONT, BACK, NO
    } cullingType = CullingType::BACK;

    std::string name;
    Vector3 minBounds, maxBounds;

    std::unique_ptr<EffectsShader> effectsShader;
    std::vector<SubMesh> submeshes;

    BaseMesh(D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    virtual ~BaseMesh() = default;

    virtual void Init() = 0;
    virtual void InitShaders() = 0;

    virtual void CalculateTangents() = 0;
    virtual void CalculateBounds() = 0;

    virtual void SubscribeInstance(Matrix m) = 0;
    virtual void Draw(const D3D11Buffer* vsMatrixes) = 0;
    virtual void DrawGeometry() = 0;
    void SetCullingType(CullingType culling);
    void SetTranslucent(bool newValue);
    void ReloadShader();

    [[nodiscard]] std::pair<Vector3, Vector3> GetBounds() noexcept
    {
        if (boundsAreDirty)
        {
            CalculateBounds();
        }
        return {minBounds, maxBounds};
    }

    virtual void RenderImGui() = 0;
};

template <class IdxType = uint16_t> requires std::is_integral_v<IdxType>
class Mesh : public BaseMesh
{
    D3D11Device* renderDevice;

protected:
    bool usesBlending;
    D3D11Buffer vertexBuffer;
    D3D11Buffer indexBuffer;

    D3D11Buffer instanceBuffer;
    std::vector<InstanceVertex> instancesModelMatrix;

    static constexpr const char* TEXTURE_BLENDING_DEFINE = "TEXTURE_BLENDING";
    static constexpr const char* DRAW_INSTANCED_DEFINE = "DRAW_INSTANCED";

    void InitShaders() override;
    void InitBuffers() override;

    void BindBuffers(const D3D11Buffer* viewProjMatrixes) override;
    void BindTextures(const SubMesh& submesh) const override;
    void BindShaders() override;

    void CalculateBounds() override;
    void CalculateTangents() override;
public:
    void Init() override;

    using IndexType = IdxType;

    Mesh(D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ~Mesh() override;

    void SetEnableBlending(bool newValue);
    bool GetBlendingEnabled() const;
    std::vector<IndexType>& GetIndexes();
    std::vector<MeshVertex>& GetVertices();
    uint32_t GetIndexCount();

    void SubscribeInstance(Matrix m) override;
    void Draw(const D3D11Buffer* viewProjBuffer) override;
    void DrawGeometry() override;

    std::vector<MeshVertex> vertices;
    std::vector<IndexType> indexes;

    void SetAllMaterialMember(const std::string& textureFilepath, std::string TexturedMaterial::* materialMember);

    void RenderImGui() override;
};
}
