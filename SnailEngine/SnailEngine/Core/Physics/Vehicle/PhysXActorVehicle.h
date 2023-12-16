#pragma once
#include "BaseVehicle.h"
#include "PxScene.h"
#include "vehicle2/PxVehicleAPI.h"

namespace Snail
{

using namespace physx;
using namespace physx::vehicle2;

struct PhysXIntegrationParams
{
    PxVehiclePhysXRoadGeometryQueryParams physxRoadGeometryQueryParams;
    PxVehiclePhysXMaterialFrictionParams physxMaterialFrictionParams[PxVehicleLimits::eMAX_NB_WHEELS];
    PxVehiclePhysXSuspensionLimitConstraintParams physxSuspensionLimitConstraintParams[PxVehicleLimits::eMAX_NB_WHEELS];
    PxTransform physxActorCMassLocalPose;
    PxVec3 physxActorBoxShapeHalfExtents;
    PxTransform physxActorBoxShapeLocalPose;
    PxTransform physxWheelShapeLocalPoses[PxVehicleLimits::eMAX_NB_WHEELS];

    void Create(
        const PxVehicleAxleDescription& axleDescription,
        const PxQueryFilterData& queryFilterData,
        PxQueryFilterCallback* queryFilterCallback,
        PxVehiclePhysXMaterialFriction* materialFrictions,
        PxU32 nbMaterialFrictions,
        PxReal defaultFriction,
        const PxTransform& physXActorCMassLocalPose,
        const PxVec3& physXActorBoxShapeHalfExtents,
        const PxTransform& physxActorBoxShapeLocalPose);

    PhysXIntegrationParams TransformAndScale(
        const PxVehicleFrame& srcFrame,
        const PxVehicleFrame& trgFrame,
        const PxVehicleScale& srcScale,
        const PxVehicleScale& trgScale) const;

    PX_FORCE_INLINE bool IsValid(const PxVehicleAxleDescription& axleDesc) const
    {
        if (!physxRoadGeometryQueryParams.isValid())
            return false;
        for (PxU32 i = 0; i < axleDesc.nbWheels; i++)
        {
            const PxU32 wheelId = axleDesc.wheelIdsInAxleOrder[i];
            if (!physxMaterialFrictionParams[wheelId].isValid())
                return false;
            if (!physxSuspensionLimitConstraintParams[wheelId].isValid())
                return false;
        }
        return true;
    }
};

struct PhysXIntegrationState
{
    PxVehiclePhysXActor physxActor; //physx actor
    PxVehiclePhysXSteerState physxSteerState;
    PxVehiclePhysXConstraints physxConstraints; //susp limit and sticky tire constraints

    PX_FORCE_INLINE void SetToDefault()
    {
        physxActor.setToDefault();
        physxSteerState.setToDefault();
        physxConstraints.setToDefault();
    }

    void Create(
        const BaseVehicleParams& baseParams,
        const PhysXIntegrationParams& physxParams,
        PxPhysics& physics,
        const PxCookingParams& params,
        const PxMaterial& defaultMaterial);

    void Destroy();
};

void SetPhysXIntegrationParams(
    const PxVehicleAxleDescription&,
    PxVehiclePhysXMaterialFriction*,
    PxU32 nbPhysXMaterialFrictions,
    PxReal physXDefaultMaterialFriction,
    PhysXIntegrationParams&);

//
//This class holds the parameters, state and logic needed to implement a vehicle that
//is using a PhysX actor to potentially interact with other objects in a PhysX scene.
//
//See BaseVehicle for more details on the snippet code design.
//
class PhysXActorVehicle : public BaseVehicle, public PxVehiclePhysXActorBeginComponent, public PxVehiclePhysXActorEndComponent,
                          public PxVehiclePhysXConstraintComponent, public PxVehiclePhysXRoadGeometrySceneQueryComponent
{
public:
    virtual bool Initialize(PxPhysics& physics, const PxCookingParams& params, PxMaterial& defaultMaterial);
    void Destroy() override;

    virtual void SetUpActor(PxScene& scene, const PxTransform& pose, const char* vehicleName);

    void getDataForPhysXActorBeginComponent(
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
        PxVehicleEngineState*& engineState) override;

    void getDataForPhysXActorEndComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleRigidBodyState*& rigidBodyState,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        PxVehicleArrayData<const PxTransform>& wheelShapeLocalPoses,
        PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
        PxVehicleArrayData<const PxVehicleWheelLocalPose>& wheelLocalPoses,
        const PxVehicleGearboxState*& gearState,
        const PxReal*& throttle,
        PxVehiclePhysXActor*& physxActor) override;

    void getDataForPhysXConstraintComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleRigidBodyState*& rigidBodyState,
        PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
        PxVehicleArrayData<const PxVehiclePhysXSuspensionLimitConstraintParams>& suspensionLimitParams,
        PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates,
        PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
        PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates,
        PxVehicleArrayData<const PxVehicleTireDirectionState>& tireDirectionStates,
        PxVehicleArrayData<const PxVehicleTireStickyState>& tireStickyStates,
        PxVehiclePhysXConstraints*& constraints) override;

    void getDataForPhysXRoadGeometrySceneQueryComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehiclePhysXRoadGeometryQueryParams*& roadGeomParams,
        PxVehicleArrayData<const PxReal>& steerResponseStates,
        const PxVehicleRigidBodyState*& rigidBodyState,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
        PxVehicleArrayData<const PxVehiclePhysXMaterialFrictionParams>& materialFrictionParams,
        PxVehicleArrayData<PxVehicleRoadGeometryState>& roadGeometryStates,
        PxVehicleArrayData<PxVehiclePhysXRoadGeometryQueryState>& physxRoadGeometryStates) override;

    //Parameters and states of the vehicle's physx integration.
    PhysXIntegrationParams mPhysXParams;
    PhysXIntegrationState mPhysXState;

    //The commands that will control the vehicle
    //
    // Note that this is not related to a PhysX actor based vehicle as such but
    // put in here to be shared by all vehicle types that will be based on this
    // class. It keeps the code simpler for the purpose of the snippets.
    //
    PxVehicleCommandState mCommandState;
};

}
