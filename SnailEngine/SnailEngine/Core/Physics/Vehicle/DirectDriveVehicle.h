#pragma once

#include <vehicle2/PxVehicleAPI.h>
#include "PhysXActorVehicle.h"

namespace Snail
{
using namespace physx;
using namespace physx::vehicle2;

struct DirectDrivetrainParams
{
    PxVehicleDirectDriveThrottleCommandResponseParams directDriveThrottleResponseParams;

    DirectDrivetrainParams TransformAndScale(
        const PxVehicleFrame& srcFrame,
        const PxVehicleFrame& trgFrame,
        const PxVehicleScale& srcScale,
        const PxVehicleScale& trgScale) const;

    PX_FORCE_INLINE bool IsValid(const PxVehicleAxleDescription& axleDesc) const
    {
        if (!directDriveThrottleResponseParams.isValid(axleDesc))
            return false;

        return true;
    }
};

struct DirectDrivetrainState
{
    PxReal directDriveThrottleResponseStates[PxVehicleLimits::eMAX_NB_WHEELS];

    PX_FORCE_INLINE void SetToDefault()
    {
        PxMemZero(this, sizeof(DirectDrivetrainState));
    }
};

//
//This class holds the parameters, state and logic needed to implement a vehicle that
//is using a direct drivetrain.
//
//See BaseVehicle for more details on the snippet code design.
//
class DirectDriveVehicle : public PhysXActorVehicle, public PxVehicleDirectDriveCommandResponseComponent,
                           public PxVehicleDirectDriveActuationStateComponent, public PxVehicleDirectDrivetrainComponent
{
public:
    bool Initialize(PxPhysics& physics, const PxCookingParams& params, PxMaterial& defaultMaterial, bool addPhysXBeginEndComponents = true);
    void Destroy() override;

    void InitComponentSequence(bool addPhysXBeginEndComponents) override;

    PxRigidBody* GetRigidBody();
    PxRigidBody* GetRigidBody() const;

    PxShape* GetMainChassisShape();

    void getDataForDirectDriveCommandResponseComponent(
        const PxVehicleAxleDescription*& axleDescription,
        PxVehicleSizedArrayData<const PxVehicleBrakeCommandResponseParams>& brakeResponseParams,
        const PxVehicleDirectDriveThrottleCommandResponseParams*& throttleResponseParams,
        const PxVehicleSteerCommandResponseParams*& steerResponseParams,
        PxVehicleSizedArrayData<const PxVehicleAckermannParams>& ackermannParams,
        const PxVehicleCommandState*& commands,
        const PxVehicleDirectDriveTransmissionCommandState*& transmissionCommands,
        const PxVehicleRigidBodyState*& rigidBodyState,
        PxVehicleArrayData<PxReal>& brakeResponseStates,
        PxVehicleArrayData<PxReal>& throttleResponseStates,
        PxVehicleArrayData<PxReal>& steerResponseStates) override;

    void getDataForDirectDriveActuationStateComponent(
        const PxVehicleAxleDescription*& axleDescription,
        PxVehicleArrayData<const PxReal>& brakeResponseStates,
        PxVehicleArrayData<const PxReal>& throttleResponseStates,
        PxVehicleArrayData<PxVehicleWheelActuationState>& actuationStates) override;

    void getDataForDirectDrivetrainComponent(
        const PxVehicleAxleDescription*& axleDescription,
        PxVehicleArrayData<const PxReal>& brakeResponseStates,
        PxVehicleArrayData<const PxReal>& throttleResponseStates,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
        PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
        PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates) override;

    //Parameters and states of the vehicle's direct drivetrain.
    DirectDrivetrainParams mDirectDriveParams;
    DirectDrivetrainState mDirectDriveState;

    //The commands that will control the vehicle's transmission
    PxVehicleDirectDriveTransmissionCommandState mTransmissionCommandState;
};
}
