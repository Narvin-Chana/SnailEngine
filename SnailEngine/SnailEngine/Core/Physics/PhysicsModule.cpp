#include "stdafx.h"
#include "PhysicsModule.h"

#include <PxPhysicsAPI.h>
#include <cooking/PxCooking.h>

#include "Callbacks/ContactCallback.h"

using namespace physx;

namespace Snail
{
void PhysicsModule::Init()
{
    foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback));
    dispatcher.reset(PxDefaultCpuDispatcherCreate(2));

#ifdef _DEBUG
    transport.reset(PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10));

    pvd.reset(PxCreatePvd(*foundation));
    if (!pvd->connect(*transport, PxPvdInstrumentationFlag::eALL))
    {
        LOGF(Logger::WARN, "Couldn't connect to PVD");
    }

    physics.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, pvd.get()));
#else
    physics.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true));
#endif

    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = dispatcher.get();
    sceneDesc.filterShader = FilterShader;
    static ContactCallback callback;
    sceneDesc.contactModifyCallback = &callback;
    sceneDesc.simulationEventCallback = &callback;
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    scene.reset(physics->createScene(sceneDesc));

#ifdef _DEBUG
    if (PxPvdSceneClient* pvdClient = scene->getScenePvdClient(); pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    if (!PxInitExtensions(*physics, pvd.get()))
    {
        LOGF(Logger::WARN, "PhysX extensions did not init properly");
    }
#endif

    vehicle2::PxInitVehicleExtension(*foundation);
    defaultMaterial.reset(physics->createMaterial(0.8f, 0.8f, 0.2f));

    materialFrictions[0].friction = 1.0f;
    materialFrictions[0].material = defaultMaterial.get();
    defaultMaterialFriction = 1.0f;
    nbMaterialFrictions = 1;
}

void PhysicsModule::Update(const float dt)
{
    scene->simulate(dt);
    scene->fetchResults(true);
}

float PhysicsModule::RaycastDistance(const Vector3& position, const Vector3& direction)
{
    const PxVec3 origin{ position.x, position.y, position.z };
    const PxVec3 unitDir{ direction.x, direction.y, direction.z };
    const PxReal maxDistance = 10000.0f;
    PxRaycastBuffer hit;

    // Raycast against all static & dynamic objects (no filtering)
    // The main result from this call is the closest hit, stored in the 'hit.block' structure
    if (scene->raycast(origin, unitDir, maxDistance, hit))
    {
        return hit.block.distance;
    }

    return std::numeric_limits<float>::max();
}

PhysicsModule::~PhysicsModule()
{
    vehicle2::PxCloseVehicleExtension();
    PxCloseExtensions();
#ifdef _DEBUG
    
#endif
}
}
