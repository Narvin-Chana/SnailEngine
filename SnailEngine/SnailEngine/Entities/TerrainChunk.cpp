#include "stdafx.h"
#include "TerrainChunk.h"

#include "Terrain.h"
#include "Core/WindowsEngine.h"
#include "Core/Physics/DynamicPhysicsObject.h"
#include "Core/Physics/StaticPhysicsObject.h"

namespace Snail
{
TerrainChunk::TerrainChunk(const Params& params)
    : Entity(params)
    , chunkSize{params.chunkSize}
{
}

void TerrainChunk::InitPhysics()
{
    if (mesh && physicsObject == nullptr)
    {
        Transform _worldTransform = Entity::GetWorldTransform();
        if (auto* shape = static_cast<TerrainMesh*>(mesh)->GetPhysicsShape(chunkSize); shape)
        {
            physicsObject = std::make_unique<StaticPhysicsObject>(shape, _worldTransform);
        }
    }
}

void TerrainChunk::PrepareShadows()
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->SetNoCulling();
}

}
