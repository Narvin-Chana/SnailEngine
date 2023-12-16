#include "stdafx.h"
#include "PhysXActorVehicle.h"

#include "Core/Physics/Callbacks/VehicleQueryCallback.h"

namespace Snail
{

void PhysXIntegrationParams::Create(
    const PxVehicleAxleDescription& axleDescription,
    const PxQueryFilterData& queryFilterData,
    PxQueryFilterCallback* queryFilterCallback,
    PxVehiclePhysXMaterialFriction* materialFrictions,
    const PxU32 nbMaterialFrictions,
    const PxReal defaultFriction,
    const PxTransform& actorCMassLocalPose,
    const PxVec3& actorBoxShapeHalfExtents,
    const PxTransform& actorBoxShapeLocalPose)
{
    physxRoadGeometryQueryParams.roadGeometryQueryType = PxVehiclePhysXRoadGeometryQueryType::eRAYCAST;
    physxRoadGeometryQueryParams.defaultFilterData = queryFilterData;
    physxRoadGeometryQueryParams.filterCallback = queryFilterCallback;
    physxRoadGeometryQueryParams.filterDataEntries = nullptr;

    for (PxU32 i = 0; i < axleDescription.nbWheels; i++)
    {
        const PxU32 wheelId = axleDescription.wheelIdsInAxleOrder[i];
        physxMaterialFrictionParams[wheelId].defaultFriction = defaultFriction;
        physxMaterialFrictionParams[wheelId].materialFrictions = materialFrictions;
        physxMaterialFrictionParams[wheelId].nbMaterialFrictions = nbMaterialFrictions;

        physxSuspensionLimitConstraintParams[wheelId].restitution = 0.0f;
        physxSuspensionLimitConstraintParams[wheelId].directionForSuspensionLimitConstraint =
        PxVehiclePhysXSuspensionLimitConstraintParams::eROAD_GEOMETRY_NORMAL;

        physxWheelShapeLocalPoses[wheelId] = PxTransform(PxIdentity);
    }

    physxActorCMassLocalPose = actorCMassLocalPose;
    physxActorBoxShapeHalfExtents = actorBoxShapeHalfExtents;
    physxActorBoxShapeLocalPose = actorBoxShapeLocalPose;
}

PhysXIntegrationParams PhysXIntegrationParams::TransformAndScale(
    const PxVehicleFrame& srcFrame,
    const PxVehicleFrame& trgFrame,
    const PxVehicleScale& srcScale,
    const PxVehicleScale& trgScale) const
{
    PhysXIntegrationParams r = *this;
    r.physxRoadGeometryQueryParams = physxRoadGeometryQueryParams.transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    for (PxU32 i = 0; i < PxVehicleLimits::eMAX_NB_WHEELS; i++)
    {
        r.physxSuspensionLimitConstraintParams[i] = physxSuspensionLimitConstraintParams[i].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    }
    r.physxActorCMassLocalPose = PxVehicleTransformFrameToFrame(srcFrame, trgFrame, srcScale, trgScale, physxActorCMassLocalPose);
    r.physxActorBoxShapeHalfExtents = PxVehicleTransformFrameToFrame(srcFrame, trgFrame, srcScale, trgScale, physxActorBoxShapeHalfExtents);
    r.physxActorBoxShapeLocalPose = PxVehicleTransformFrameToFrame(srcFrame, trgFrame, srcScale, trgScale, physxActorBoxShapeLocalPose);
    return r;
}

void PhysXIntegrationState::Create(
    const BaseVehicleParams& baseParams,
    const PhysXIntegrationParams& physxParams,
    PxPhysics& physics,
    const PxCookingParams& params,
    const PxMaterial& defaultMaterial)
{
    SetToDefault();

    //physxActor needs to be populated with an actor and its shapes.
    {
        const PxVehiclePhysXRigidActorParams physxActorParams(baseParams.rigidBodyParams, nullptr);
        const PxBoxGeometry boxGeom(physxParams.physxActorBoxShapeHalfExtents);
        const PxVehiclePhysXRigidActorShapeParams physxActorShapeParams(boxGeom,
            physxParams.physxActorBoxShapeLocalPose,
            defaultMaterial,
            PxShapeFlags(PxShapeFlag::eSIMULATION_SHAPE),
            PxFilterData(),
            PxFilterData());

        const PxVehiclePhysXWheelParams physxWheelParams(baseParams.axleDescription, baseParams.wheelParams);
        const PxVehiclePhysXWheelShapeParams physxWheelShapeParams(defaultMaterial,
            PxShapeFlags(),
            PxFilterData(),
            PxFilterData());

        PxVehiclePhysXActorCreate(baseParams.frame,
            physxActorParams,
            physxParams.physxActorCMassLocalPose,
            physxActorShapeParams,
            physxWheelParams,
            physxWheelShapeParams,
            physics,
            params,
            physxActor);
    }

    //physxConstraints needs to be populated with constraints.
    PxVehicleConstraintsCreate(baseParams.axleDescription, physics, *physxActor.rigidBody, physxConstraints);
}

void PhysXIntegrationState::Destroy()
{
    PxVehicleConstraintsDestroy(physxConstraints);
    PxVehiclePhysXActorDestroy(physxActor);
}

void SetPhysXIntegrationParams(
    const PxVehicleAxleDescription& axleDescription,
    PxVehiclePhysXMaterialFriction* physXMaterialFrictions,
    PxU32 nbPhysXMaterialFrictions,
    PxReal physXDefaultMaterialFriction,
    PhysXIntegrationParams& physXParams)
{
    // This defines how the vehicle's QueryFilter responds to other objects
    static VehicleQueryCallback callback;
    // The first flag defines that the prefilter callback should be executed no matter what to potentially filter certain shapes
    // (in our case triggers since we do not want them to have a physical collision with our vehicle)
    // The other two flags (eSTATIC and eDYNAMIC) define the types of shapes that this vehicle can collide with in a SceneQuery
    const PxQueryFilterData queryFilterData = PxQueryFilterData(PxFilterData{},
        PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC);
    PxQueryFilterCallback* queryFilterCallback = &callback;

    // The physx integration params are hardcoded rather than loaded from file.
    // Either could work since I imported the rapidjson serialisation code from the PhysX snippets aswell.
    const PxTransform physxActorCMassLocalPose(PxVec3(0.0f, 0.55f, 1.594f), PxQuat(PxIdentity));
    const PxVec3 physxActorBoxShapeHalfExtents(0.84097f, 0.65458f, 2.46971f);
    const PxTransform physxActorBoxShapeLocalPose(PxVec3(0.0f,1.f, 1.37003f), PxQuat(PxIdentity));

    physXParams.Create(axleDescription,
        queryFilterData,
        queryFilterCallback,
        physXMaterialFrictions,
        nbPhysXMaterialFrictions,
        physXDefaultMaterialFriction,
        physxActorCMassLocalPose,
        physxActorBoxShapeHalfExtents,
        physxActorBoxShapeLocalPose);
}

bool PhysXActorVehicle::Initialize(PxPhysics& physics, const PxCookingParams& params, PxMaterial& defaultMaterial)
{
    mCommandState.setToDefault();

    if (!BaseVehicle::Initialize())
        return false;

    if (!mPhysXParams.IsValid(mBaseParams.axleDescription))
        return false;

    mPhysXState.Create(mBaseParams, mPhysXParams, physics, params, defaultMaterial);

    return true;
}

void PhysXActorVehicle::Destroy()
{
    mPhysXState.Destroy();

    BaseVehicle::Destroy();
}

void PhysXActorVehicle::SetUpActor(PxScene& scene, const PxTransform& pose, const char* vehicleName)
{
    //Give the vehicle a start pose by appylying a pose to the PxRigidDynamic associated with the vehicle. 
    //This vehicle has components that are configured to read the pose from the PxRigidDynamic 
    //at the start of the vehicle simulation update and to write back an updated pose at the end of the 
    //vehicle simulation update. This allows PhysX to manage any collisions that might happen in-between 
    //each vehicle update. This is not essential but it is anticipated that this will be a typical component 
    //configuration. 
    mPhysXState.physxActor.rigidBody->setGlobalPose(pose);

    //Add the physx actor to the physx scene.
    //As described above, a vehicle may be coupled to a physx scene or it can be simulated without any reference to 
    //to a PxRigidDynamic or PxScene. This snippet vehicle employs a configuration that includes coupling to a PxScene and a 
    //PxRigidDynamic. This being the case, the PxRigidDynamic associated with the vehicle needs to be added to the 
    //PxScene instance.
    scene.addActor(*mPhysXState.physxActor.rigidBody);

    //Give the physx actor a name to help identification in PVD
    mPhysXState.physxActor.rigidBody->setName(vehicleName);
}

void PhysXActorVehicle::getDataForPhysXActorBeginComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehicleCommandState*& commands,
    const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
    const PxVehicleGearboxParams*& gearParams,
    const PxVehicleGearboxState*& gearState,
    const PxVehicleEngineParams*& engineParams,
    PxVehiclePhysXActor*& physxActor,
    PxVehiclePhysXSteerState*& physxSteerState,
    PxVehiclePhysXConstraints*& physxConstraints,
    PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
    PxVehicleEngineState*& engineState)
{
    axleDescription = &mBaseParams.axleDescription;
    commands = &mCommandState;
    physxActor = &mPhysXState.physxActor;
    physxSteerState = &mPhysXState.physxSteerState;
    physxConstraints = &mPhysXState.physxConstraints;
    rigidBodyState = &mBaseState.rigidBodyState;
    wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);

    transmissionCommands = nullptr;
    gearParams = nullptr;
    gearState = nullptr;
    engineParams = nullptr;
    engineState = nullptr;
}

void PhysXActorVehicle::getDataForPhysXActorEndComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxTransform>& wheelShapeLocalPoses,
    PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
    PxVehicleArrayData<const PxVehicleWheelLocalPose>& wheelLocalPoses,
    const PxVehicleGearboxState*& gearState,
    const PxReal*& throttle,
    PxVehiclePhysXActor*& physxActor)
{
    axleDescription = &mBaseParams.axleDescription;
    rigidBodyState = &mBaseState.rigidBodyState;
    wheelParams.setData(mBaseParams.wheelParams);
    wheelShapeLocalPoses.setData(mPhysXParams.physxWheelShapeLocalPoses);
    wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);
    wheelLocalPoses.setData(mBaseState.wheelLocalPoses);
    physxActor = &mPhysXState.physxActor;

    gearState = nullptr;
    throttle = &mCommandState.throttle;
}

void PhysXActorVehicle::getDataForPhysXConstraintComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
    PxVehicleArrayData<const PxVehiclePhysXSuspensionLimitConstraintParams>& suspensionLimitParams,
    PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates,
    PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
    PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates,
    PxVehicleArrayData<const PxVehicleTireDirectionState>& tireDirectionStates,
    PxVehicleArrayData<const PxVehicleTireStickyState>& tireStickyStates,
    PxVehiclePhysXConstraints*& constraints)
{
    axleDescription = &mBaseParams.axleDescription;
    rigidBodyState = &mBaseState.rigidBodyState;
    suspensionParams.setData(mBaseParams.suspensionParams);
    suspensionLimitParams.setData(mPhysXParams.physxSuspensionLimitConstraintParams);
    suspensionStates.setData(mBaseState.suspensionStates);
    suspensionComplianceStates.setData(mBaseState.suspensionComplianceStates);
    wheelRoadGeomStates.setData(mBaseState.roadGeomStates);
    tireDirectionStates.setData(mBaseState.tireDirectionStates);
    tireStickyStates.setData(mBaseState.tireStickyStates);
    constraints = &mPhysXState.physxConstraints;
}

void PhysXActorVehicle::getDataForPhysXRoadGeometrySceneQueryComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehiclePhysXRoadGeometryQueryParams*& roadGeomParams,
    PxVehicleArrayData<const PxReal>& steerResponseStates,
    const PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
    PxVehicleArrayData<const PxVehiclePhysXMaterialFrictionParams>& materialFrictionParams,
    PxVehicleArrayData<PxVehicleRoadGeometryState>& roadGeometryStates,
    PxVehicleArrayData<PxVehiclePhysXRoadGeometryQueryState>& physxRoadGeometryStates)
{
    axleDescription = &mBaseParams.axleDescription;
    roadGeomParams = &mPhysXParams.physxRoadGeometryQueryParams;
    steerResponseStates.setData(mBaseState.steerCommandResponseStates);
    rigidBodyState = &mBaseState.rigidBodyState;
    wheelParams.setData(mBaseParams.wheelParams);
    suspensionParams.setData(mBaseParams.suspensionParams);
    materialFrictionParams.setData(mPhysXParams.physxMaterialFrictionParams);
    roadGeometryStates.setData(mBaseState.roadGeomStates);
    physxRoadGeometryStates.setEmpty();
}

}
