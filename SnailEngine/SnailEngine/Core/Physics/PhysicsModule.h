#pragma once
#include <PxPhysicsAPI.h>

#include "PhysXAllocator.h"
#include "Core/Mesh/Mesh.h"

namespace physx
{
class PxFoundation;
class PxDefaultCpuDispatcher;
class PxPvd;
class PxPhysics;
class PxScene;
class PxMaterial;
}

inline std::mutex PhysxMutex;

namespace Snail
{
struct PhysicsModule
{
    //The mapping between PxMaterial and friction.
    physx::PxU32 nbMaterialFrictions;
    physx::PxReal defaultMaterialFriction;
    physx::vehicle2::PxVehiclePhysXMaterialFriction materialFrictions[16];

    physx::PxDefaultAllocator allocator{};
    physx::PxDefaultErrorCallback errorCallback{};

    // THE ORDER OF THESE DECLARATIONS MUST STAY THE SAME!!
    PhysXUniquePtr<physx::PxFoundation> foundation;
    PhysXUniquePtr<physx::PxCudaContextManager> CUDAContextManager;
    PhysXUniquePtr<physx::PxDefaultCpuDispatcher> dispatcher;
#ifdef _DEBUG
    PhysXUniquePtr<physx::PxPvdTransport> transport;
    PhysXUniquePtr<physx::PxPvd> pvd;
#endif
    PhysXUniquePtr<physx::PxPhysics> physics;
    PhysXUniquePtr<physx::PxScene> scene;
    PhysXUniquePtr<physx::PxMaterial> defaultMaterial;

    void Init();
    void Update(float dt);

    float RaycastDistance(const Vector3& position, const Vector3& direction);

    ~PhysicsModule();

    template <class IdxType> requires std::is_integral_v<IdxType>
    physx::PxShape* GenerateMeshConvexShape(const Mesh<IdxType>*, const Vector3& transformScale = Vector3::One);

    template <class IdxType> requires std::is_integral_v<IdxType>
    physx::PxShape* GenerateMeshTriangleShape(const Mesh<IdxType>*, const Vector3& transformScale = Vector3::One);
};

template <class IdxType> requires std::is_integral_v<IdxType>
physx::PxShape* PhysicsModule::GenerateMeshConvexShape(const Mesh<IdxType>* mesh, const Vector3& transformScale)
{
    using namespace physx;
    PxTolerancesScale scale;
    PxCookingParams params(scale);

    std::vector<PxVec3> verts;
    Matrix scaleMat = Matrix::CreateScale(transformScale);
    std::ranges::transform(mesh->vertices, std::back_inserter(verts), [&scaleMat](const MeshVertex& meshVertex)
        {
            Vector3 pos = Vector3::Transform(meshVertex.position, scaleMat);
            return PxVec3{ pos.x, pos.y, pos.z };
        });

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = static_cast<PxU32>(verts.size());
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = verts.data();
    meshDesc.flags.raise(PxConvexFlag::eCOMPUTE_CONVEX);

    PxDefaultMemoryOutputStream writeBuffer;
    PxConvexMeshCookingResult::Enum result;
    if (!PxCookConvexMesh(params, meshDesc, writeBuffer, &result))
    {
        LOG(Logger::ERROR, "PhysX mesh cooking failed");
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

    PhysXUniquePtr<PxConvexMesh> triangleMesh;
    triangleMesh.reset(physics->createConvexMesh(readBuffer));
    return physics->createShape(PxConvexMeshGeometry(triangleMesh.get()), *defaultMaterial);
}

template <class IdxType> requires std::is_integral_v<IdxType>
physx::PxShape* PhysicsModule::GenerateMeshTriangleShape(const Mesh<IdxType>* mesh, const Vector3& transformScale)
{
    using namespace physx;
    PxTolerancesScale scale;
    PxCookingParams params(scale);

    std::vector<PxVec3> verts;
    Matrix scaleMat = Matrix::CreateScale(transformScale);
    std::ranges::transform(mesh->vertices, std::back_inserter(verts), [&scaleMat](const MeshVertex& meshVertex)
        {
            Vector3 pos = Vector3::Transform(meshVertex.position, scaleMat);
            return PxVec3{ pos.x, pos.y, pos.z };
        });

    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = static_cast<PxU32>(verts.size());
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = verts.data();

    meshDesc.triangles.count = static_cast<PxU32>(mesh->indexes.size() / 3);
    if constexpr (std::is_same_v<IdxType, uint16_t>)
        meshDesc.flags.raise(PxMeshFlag::e16_BIT_INDICES);

    meshDesc.triangles.stride = 3 * sizeof(IdxType);
    meshDesc.triangles.data = mesh->indexes.data();

    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    if (!PxCookTriangleMesh(params, meshDesc, writeBuffer, &result))
    {
        LOG(Logger::ERROR, "Failed to cook triangle mesh");
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

    PhysXUniquePtr<PxTriangleMesh> triangleMesh;
    triangleMesh.reset(physics->createTriangleMesh(readBuffer));
    return physics->createShape(PxTriangleMeshGeometry(triangleMesh.get()), *defaultMaterial);
}

}
