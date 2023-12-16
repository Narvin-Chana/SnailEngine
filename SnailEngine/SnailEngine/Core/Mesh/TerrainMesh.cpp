#include "stdafx.h"
#include "TerrainMesh.h"

#include "Core/WindowsEngine.h"
using namespace physx;

namespace Snail
{
TerrainMesh::TerrainMesh()
{
    static const PhysicsModule& physicsModule = WindowsEngine::GetModule<PhysicsModule>();
    physXMats[0] = physicsModule.physics->createMaterial(0, 0, 0);

    static auto roadCallback = []{};
    physXMats[0]->userData = roadCallback;
    physXMats[1] = physicsModule.physics->createMaterial(0, 0, 0);

    static auto grassCallback = []
    {
        auto& gm = WindowsEngine::GetInstance().GetModule<GameManager>();
        if (const auto vehicle = gm.GetVehicle())
        {
            vehicle->SetOnGrass(true);
        }
    };
    physXMats[1]->userData = grassCallback;
}

TerrainMesh::~TerrainMesh()
{
    physXMats[0]->release();
    physXMats[1]->release();
}

void TerrainMesh::PopulateHeightField(const uint32_t w, const uint32_t h)
{
    if (w == 0 || h == 0)
    {
        width = height = static_cast<uint32_t>(std::sqrt(vertices.size()));
        return;
    }
    width = w;
    height = h;
}

void TerrainMesh::SortVertices()
{
    std::ranges::sort(vertices,
                      [](const auto& a, const auto& b){
                          return a.position.z < b.position.z;
                      });

    for (uint32_t row = 0; row < height; ++row)
    {
        std::sort(vertices.begin() + row * width,
                  vertices.begin() + row * width + width,
                  [](const auto& a, const auto& b){
                      return a.position.x < b.position.x;
                  });
    }
}

PxShape* TerrainMesh::GetPhysicsShape(const Vector3& size) const
{
    static const PhysicsModule& physicsModule = WindowsEngine::GetModule<PhysicsModule>();

    // min height will always be zero to align all the terrain chunks
    const PxReal minHeight = 0;
    PxReal maxHeight = -PX_MAX_F32;
    for (uint32_t x = 0; x < width; ++x)
    {
        for (uint32_t y = 0; y < height; ++y)
        {
            maxHeight = std::max(maxHeight, vertices[x + y * width].position.y);
        }
    }

    // the full height of the terrain
    const PxReal deltaHeight = maxHeight - minHeight;
    // the max value the height can be (in float)
    constexpr PxReal quantization = static_cast<PxReal>(0x7fff);

    std::vector<PxHeightFieldSample> heightFieldSampleData;
    heightFieldSampleData.resize(width * height);

    for (uint32_t x = 0; x < width; ++x)
    {
        for (uint32_t y = 0; y < height; ++y)   
        {
            // cols and rows are flipped in the sample data. dont know why but it works this way now
            heightFieldSampleData[y + x * height].height = static_cast<PxI16>(quantization * ((vertices[x + y * width].position.y - minHeight) / deltaHeight));

            if (materialIds[x + y * width] == 1)
            {
                heightFieldSampleData[y + x * height].materialIndex0.setBit();
                heightFieldSampleData[y + x * height].materialIndex1.setBit();
            }
        }
    }

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbColumns = height;
    hfDesc.nbRows = width;
    hfDesc.samples.data = heightFieldSampleData.data();
    hfDesc.samples.stride = sizeof(PxHeightFieldSample);

    PxDefaultMemoryOutputStream stream;
    PxCookHeightField(hfDesc, stream);
    

    PhysXUniquePtr<PxHeightField> field;
    field.reset(PxCreateHeightField(hfDesc, physicsModule.physics->getPhysicsInsertionCallback()));
    PxHeightFieldGeometry heightMapGeom(field.get(), {});

    // We do -1 because the last rows/column doesn't exist is closes the second to last one
    heightMapGeom.columnScale = size.z / (hfDesc.nbColumns - 1);
    heightMapGeom.rowScale = size.x / (hfDesc.nbRows - 1);
    
    // Terrain heights are from 0 to 65535. wee need them between 0 and size.y / maxHeight (the highest point)
    heightMapGeom.heightScale = size.y / std::numeric_limits<PxI16>::max() * maxHeight;


    return physicsModule.physics->createShape(heightMapGeom, physXMats, 2);
}

void TerrainMesh::SetUVScale(const Vector2& uvScale)
{
    for (SubMesh& subMesh : submeshes)
    {
        TexturedMaterial subMat = subMesh.GetMaterial();
        subMat.material.uvScale = uvScale;
        subMesh.SetMaterial(subMat);
    }
}

void TerrainMesh::SetPrimaryBlendUVScale(const Vector2& blendUvScale)
{
    for (SubMesh& subMesh : submeshes)
    {
        TexturedMaterial subMat = subMesh.GetMaterial();
        subMat.material.primaryBlendUvScale = blendUvScale;
        subMesh.SetMaterial(subMat);
    }
}

void TerrainMesh::SetSecondaryBlendUVScale(const Vector2& blendUvScale)
{
    for (SubMesh& subMesh : submeshes)
    {
        TexturedMaterial subMat = subMesh.GetMaterial();
        subMat.material.secondaryBlendUvScale = blendUvScale;
        subMesh.SetMaterial(subMat);
    }
}
}
