#include "stdafx.h"
#include "BaseVehicle.h"

namespace Snail
{

BaseVehicleParams BaseVehicleParams::TransformAndScale(
    const PxVehicleFrame& srcFrame,
    const PxVehicleFrame& trgFrame,
    const PxVehicleScale& srcScale,
    const PxVehicleScale& trgScale) const
{
    BaseVehicleParams r = *this;
    r.axleDescription = axleDescription;
    r.frame = trgFrame;
    r.scale = trgScale;

    r.suspensionStateCalculationParams = suspensionStateCalculationParams.transformAndScale(srcFrame, trgFrame, srcScale, trgScale);

    r.brakeResponseParams[0] = brakeResponseParams[0].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    r.brakeResponseParams[1] = brakeResponseParams[1].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    r.steerResponseParams = steerResponseParams.transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    r.ackermannParams[0] = ackermannParams[0].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);

    for (PxU32 i = 0; i < r.axleDescription.nbWheels; i++)
    {
        const PxU32 wheelId = r.axleDescription.wheelIdsInAxleOrder[i];

        r.suspensionParams[wheelId] = suspensionParams[wheelId].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
        r.suspensionComplianceParams[wheelId] = suspensionComplianceParams[wheelId].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
        r.suspensionForceParams[wheelId] = suspensionForceParams[wheelId].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);

        r.tireForceParams[wheelId] = tireForceParams[wheelId].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);

        r.wheelParams[wheelId] = wheelParams[wheelId].transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    }

    r.rigidBodyParams = rigidBodyParams.transformAndScale(srcFrame, trgFrame, srcScale, trgScale);

    return r;
}

bool BaseVehicle::Initialize()
{
    if (!mBaseParams.IsValid())
        return false;

    //Set the base state to default.
    mBaseState.SetToDefault();

    return true;
}

void BaseVehicle::Step(const PxReal dt, const PxVehicleSimulationContext& context)
{
    mComponentSequence.update(dt, context);
}

void BaseVehicle::getDataForRigidBodyComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehicleRigidBodyParams*& rigidBodyParams,
    PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces,
    PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
    const PxVehicleAntiRollTorque*& antiRollTorque,
    PxVehicleRigidBodyState*& rigidBodyState)
{
    axleDescription = &mBaseParams.axleDescription;
    rigidBodyParams = &mBaseParams.rigidBodyParams;
    suspensionForces.setData(mBaseState.suspensionForces);
    tireForces.setData(mBaseState.tireForces);
    antiRollTorque = nullptr;
    rigidBodyState = &mBaseState.rigidBodyState;
}

void BaseVehicle::getDataForSuspensionComponent(
    const PxVehicleAxleDescription*& axleDescription,
    const PxVehicleRigidBodyParams*& rigidBodyParams,
    const PxVehicleSuspensionStateCalculationParams*& suspensionStateCalculationParams,
    PxVehicleArrayData<const PxReal>& steerResponseStates,
    const PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
    PxVehicleArrayData<const PxVehicleSuspensionComplianceParams>& suspensionComplianceParams,
    PxVehicleArrayData<const PxVehicleSuspensionForceParams>& suspensionForceParams,
    PxVehicleSizedArrayData<const PxVehicleAntiRollForceParams>& antiRollForceParams,
    PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates,
    PxVehicleArrayData<PxVehicleSuspensionState>& suspensionStates,
    PxVehicleArrayData<PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
    PxVehicleArrayData<PxVehicleSuspensionForce>& suspensionForces,
    PxVehicleAntiRollTorque*& antiRollTorque)
{
    axleDescription = &mBaseParams.axleDescription;
    rigidBodyParams = &mBaseParams.rigidBodyParams;
    suspensionStateCalculationParams = &mBaseParams.suspensionStateCalculationParams;
    steerResponseStates.setData(mBaseState.steerCommandResponseStates);
    rigidBodyState = &mBaseState.rigidBodyState;
    wheelParams.setData(mBaseParams.wheelParams);
    suspensionParams.setData(mBaseParams.suspensionParams);
    suspensionComplianceParams.setData(mBaseParams.suspensionComplianceParams);
    suspensionForceParams.setData(mBaseParams.suspensionForceParams);
    antiRollForceParams.setEmpty();
    wheelRoadGeomStates.setData(mBaseState.roadGeomStates);
    suspensionStates.setData(mBaseState.suspensionStates);
    suspensionComplianceStates.setData(mBaseState.suspensionComplianceStates);
    suspensionForces.setData(mBaseState.suspensionForces);
    antiRollTorque = nullptr;
}

void BaseVehicle::getDataForTireComponent(
    const PxVehicleAxleDescription*& axleDescription,
    PxVehicleArrayData<const PxReal>& steerResponseStates,
    const PxVehicleRigidBodyState*& rigidBodyState,
    PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
    PxVehicleArrayData<const PxVehicleTireForceParams>& tireForceParams,
    PxVehicleArrayData<const PxVehicleRoadGeometryState>& roadGeomStates,
    PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates,
    PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
    PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces,
    PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1DStates,
    PxVehicleArrayData<PxVehicleTireGripState>& tireGripStates,
    PxVehicleArrayData<PxVehicleTireDirectionState>& tireDirectionStates,
    PxVehicleArrayData<PxVehicleTireSpeedState>& tireSpeedStates,
    PxVehicleArrayData<PxVehicleTireSlipState>& tireSlipStates,
    PxVehicleArrayData<PxVehicleTireCamberAngleState>& tireCamberAngleStates,
    PxVehicleArrayData<PxVehicleTireStickyState>& tireStickyStates,
    PxVehicleArrayData<PxVehicleTireForce>& tireForces)
{
    axleDescription = &mBaseParams.axleDescription;
    steerResponseStates.setData(mBaseState.steerCommandResponseStates);
    rigidBodyState = &mBaseState.rigidBodyState;
    actuationStates.setData(mBaseState.actuationStates);
    wheelParams.setData(mBaseParams.wheelParams);
    suspensionParams.setData(mBaseParams.suspensionParams);
    tireForceParams.setData(mBaseParams.tireForceParams);
    roadGeomStates.setData(mBaseState.roadGeomStates);
    suspensionStates.setData(mBaseState.suspensionStates);
    suspensionComplianceStates.setData(mBaseState.suspensionComplianceStates);
    suspensionForces.setData(mBaseState.suspensionForces);
    wheelRigidBody1DStates.setData(mBaseState.wheelRigidBody1dStates);
    tireGripStates.setData(mBaseState.tireGripStates);
    tireDirectionStates.setData(mBaseState.tireDirectionStates);
    tireSpeedStates.setData(mBaseState.tireSpeedStates);
    tireSlipStates.setData(mBaseState.tireSlipStates);
    tireCamberAngleStates.setData(mBaseState.tireCamberAngleStates);
    tireStickyStates.setData(mBaseState.tireStickyStates);
    tireForces.setData(mBaseState.tireForces);
}

void BaseVehicle::getDataForWheelComponent(
    const PxVehicleAxleDescription*& axleDescription,
    PxVehicleArrayData<const PxReal>& steerResponseStates,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
    PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
    PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates,
    PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
    PxVehicleArrayData<const PxVehicleTireSpeedState>& tireSpeedStates,
    PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
    PxVehicleArrayData<PxVehicleWheelLocalPose>& wheelLocalPoses)
{
    axleDescription = &mBaseParams.axleDescription;
    steerResponseStates.setData(mBaseState.steerCommandResponseStates);
    wheelParams.setData(mBaseParams.wheelParams);
    suspensionParams.setData(mBaseParams.suspensionParams);
    actuationStates.setData(mBaseState.actuationStates);
    suspensionStates.setData(mBaseState.suspensionStates);
    suspensionComplianceStates.setData(mBaseState.suspensionComplianceStates);
    tireSpeedStates.setData(mBaseState.tireSpeedStates);
    wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);
    wheelLocalPoses.setData(mBaseState.wheelLocalPoses);
}

}
