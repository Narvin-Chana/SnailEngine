#pragma once
#include "Mesh.h"
#include "Core/Physics/PhysXAllocator.h"

namespace Snail
{
class TerrainMesh : public Mesh<uint32_t>
{
    PhysXUniquePtr<physx::PxMaterial> physXmat[2];
    physx::PxMaterial*  physXMats[2];
public:
    std::vector<int> materialIds;
    uint32_t width = 0, height = 0;

    TerrainMesh();
    ~TerrainMesh() override;
    void PopulateHeightField(uint32_t w, uint32_t h);
    void SortVertices();
    physx::PxShape* GetPhysicsShape(const Vector3& size) const;
    void SetUVScale(const Vector2& uvScale);

    void SetPrimaryBlendUVScale(const Vector2& blendUvScale);
    void SetSecondaryBlendUVScale(const Vector2& blendUvScale);
};
}
