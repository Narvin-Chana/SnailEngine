#pragma once

#include "PhysXActorVehicle.h"
#include "vehicle2/PxVehicleAPI.h"
#include <array>

namespace Snail
{

using namespace physx;
using namespace physx::vehicle2;

struct EngineDrivetrainParams
{
    PxVehicleAutoboxParams autoboxParams;
    PxVehicleClutchCommandResponseParams clutchCommandResponseParams;
    PxVehicleEngineParams engineParams;
    PxVehicleGearboxParams gearBoxParams;
    PxVehicleMultiWheelDriveDifferentialParams multiWheelDifferentialParams;
    PxVehicleFourWheelDriveDifferentialParams fourWheelDifferentialParams;
    PxVehicleTankDriveDifferentialParams tankDifferentialParams;
    PxVehicleClutchParams clutchParams;

    EngineDrivetrainParams TransformAndScale(
        const PxVehicleFrame& srcFrame,
        const PxVehicleFrame& trgFrame,
        const PxVehicleScale& srcScale,
        const PxVehicleScale& trgScale) const;

    PX_FORCE_INLINE bool IsValid(const PxVehicleAxleDescription& axleDesc) const
    {
        if (!autoboxParams.isValid(gearBoxParams))
            return false;
        if (!clutchCommandResponseParams.isValid())
            return false;
        if (!engineParams.isValid())
            return false;
        if (!gearBoxParams.isValid())
            return false;
        if (!multiWheelDifferentialParams.isValid(axleDesc))
            return false;
        if (!fourWheelDifferentialParams.isValid(axleDesc))
            return false;
        if (!tankDifferentialParams.isValid(axleDesc))
            return false;
        if (!clutchParams.isValid())
            return false;
        return true;
    }
};

struct EngineDrivetrainState
{
    PxVehicleEngineDriveThrottleCommandResponseState throttleCommandResponseState;
    PxVehicleAutoboxState autoboxState;
    PxVehicleClutchCommandResponseState clutchCommandResponseState;
    PxVehicleDifferentialState differentialState;
    PxVehicleWheelConstraintGroupState wheelConstraintGroupState;
    PxVehicleEngineState engineState;
    PxVehicleGearboxState gearboxState;
    PxVehicleClutchSlipState clutchState;

    PX_FORCE_INLINE void SetToDefault()
    {
        throttleCommandResponseState.setToDefault();
        autoboxState.setToDefault();
        clutchCommandResponseState.setToDefault();
        differentialState.setToDefault();
        wheelConstraintGroupState.setToDefault();
        engineState.setToDefault();
        gearboxState.setToDefault();
        clutchState.setToDefault();
    }
};

class EngineDriveVehicle : public PhysXActorVehicle, public PxVehicleEngineDriveCommandResponseComponent,
                           public PxVehicleFourWheelDriveDifferentialStateComponent, public PxVehicleMultiWheelDriveDifferentialStateComponent,
                           public PxVehicleTankDriveDifferentialStateComponent, public PxVehicleEngineDriveActuationStateComponent,
                           public PxVehicleEngineDrivetrainComponent
{
public:
    enum Enum
    {
        eDIFFTYPE_FOURWHEELDRIVE,
        eDIFFTYPE_MULTIWHEELDRIVE,
        eDIFFTYPE_TANKDRIVE
    };

    bool Initialize(
        PxPhysics& physics,
        const PxCookingParams& params,
        PxMaterial& defaultMaterial,
        Enum differentialType,
        bool addPhysXBeginEndComponents = true);
    void Destroy() override;

    void InitComponentSequence(bool addPhysXBeginEndComponents) override;

    PxRigidBody* GetRigidBody();
    PxRigidBody* GetRigidBody() const;

    PxShape* GetMainChassisShape();
    std::array<PxShape*, 4> GetWheelShape();

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
        PxVehicleEngineState*& engineState) override
    {
        axleDescription = &mBaseParams.axleDescription;
        commands = &mCommandState;
        physxActor = &mPhysXState.physxActor;
        physxSteerState = &mPhysXState.physxSteerState;
        physxConstraints = &mPhysXState.physxConstraints;
        rigidBodyState = &mBaseState.rigidBodyState;
        wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);

        transmissionCommands = &mTransmissionCommandState;
        gearParams = &mEngineDriveParams.gearBoxParams;
        gearState = &mEngineDriveState.gearboxState;
        engineParams = &mEngineDriveParams.engineParams;
        engineState = &mEngineDriveState.engineState;
    }

    void getDataForPhysXActorEndComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleRigidBodyState*& rigidBodyState,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        PxVehicleArrayData<const PxTransform>& wheelShapeLocalPoses,
        PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
        PxVehicleArrayData<const PxVehicleWheelLocalPose>& wheelLocalPoses,
        const PxVehicleGearboxState*& gearState,
        const PxReal*& throttle,
        PxVehiclePhysXActor*& physxActor) override
    {
        axleDescription = &mBaseParams.axleDescription;
        rigidBodyState = &mBaseState.rigidBodyState;
        wheelParams.setData(mBaseParams.wheelParams);
        wheelShapeLocalPoses.setData(mPhysXParams.physxWheelShapeLocalPoses);
        wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);
        wheelLocalPoses.setData(mBaseState.wheelLocalPoses);
        physxActor = &mPhysXState.physxActor;

        gearState = &mEngineDriveState.gearboxState;
        throttle = &mCommandState.throttle;
    }

    void getDataForEngineDriveCommandResponseComponent(
        const PxVehicleAxleDescription*& axleDescription,
        PxVehicleSizedArrayData<const PxVehicleBrakeCommandResponseParams>& brakeResponseParams,
        const PxVehicleSteerCommandResponseParams*& steerResponseParams,
        PxVehicleSizedArrayData<const PxVehicleAckermannParams>& ackermannParams,
        const PxVehicleGearboxParams*& gearboxParams,
        const PxVehicleClutchCommandResponseParams*& clutchResponseParams,
        const PxVehicleEngineParams*& engineParams,
        const PxVehicleRigidBodyState*& rigidBodyState,
        const PxVehicleEngineState*& engineState,
        const PxVehicleAutoboxParams*& autoboxParams,
        const PxVehicleCommandState*& commands,
        const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
        PxVehicleArrayData<PxReal>& brakeResponseStates,
        PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
        PxVehicleArrayData<PxReal>& steerResponseStates,
        PxVehicleGearboxState*& gearboxResponseState,
        PxVehicleClutchCommandResponseState*& clutchResponseState,
        PxVehicleAutoboxState*& autoboxState) override
    {
        axleDescription = &mBaseParams.axleDescription;
        brakeResponseParams.setDataAndCount(mBaseParams.brakeResponseParams,
            sizeof(mBaseParams.brakeResponseParams) / sizeof(PxVehicleBrakeCommandResponseParams));
        steerResponseParams = &mBaseParams.steerResponseParams;
        ackermannParams.setDataAndCount(mBaseParams.ackermannParams, sizeof(mBaseParams.ackermannParams) / sizeof(PxVehicleAckermannParams));
        gearboxParams = &mEngineDriveParams.gearBoxParams;
        clutchResponseParams = &mEngineDriveParams.clutchCommandResponseParams;
        engineParams = &mEngineDriveParams.engineParams;
        rigidBodyState = &mBaseState.rigidBodyState;
        engineState = &mEngineDriveState.engineState;
        autoboxParams = &mEngineDriveParams.autoboxParams;
        commands = &mCommandState;
        transmissionCommands = (Enum::eDIFFTYPE_TANKDRIVE == mDifferentialType) ? &mTankDriveTransmissionCommandState : &mTransmissionCommandState;
        brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
        throttleResponseState = &mEngineDriveState.throttleCommandResponseState;
        steerResponseStates.setData(mBaseState.steerCommandResponseStates);
        gearboxResponseState = &mEngineDriveState.gearboxState;
        clutchResponseState = &mEngineDriveState.clutchCommandResponseState;
        autoboxState = &mEngineDriveState.autoboxState;
    }

    void getDataForFourWheelDriveDifferentialStateComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleFourWheelDriveDifferentialParams*& differentialParams,
        PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidbody1dStates,
        PxVehicleDifferentialState*& differentialState,
        PxVehicleWheelConstraintGroupState*& wheelConstraintGroups) override
    {
        axleDescription = &mBaseParams.axleDescription;
        differentialParams = &mEngineDriveParams.fourWheelDifferentialParams;
        wheelRigidbody1dStates.setData(mBaseState.wheelRigidBody1dStates);
        differentialState = &mEngineDriveState.differentialState;
        wheelConstraintGroups = &mEngineDriveState.wheelConstraintGroupState;
    }

    void getDataForMultiWheelDriveDifferentialStateComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleMultiWheelDriveDifferentialParams*& differentialParams,
        PxVehicleDifferentialState*& differentialState) override
    {
        axleDescription = &mBaseParams.axleDescription;
        differentialParams = &mEngineDriveParams.multiWheelDifferentialParams;
        differentialState = &mEngineDriveState.differentialState;
    }

    void getDataForTankDriveDifferentialStateComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleTankDriveTransmissionCommandState*& tankDriveTransmissionCommands,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        const PxVehicleTankDriveDifferentialParams*& differentialParams,
        PxVehicleDifferentialState*& differentialState,
        PxVehicleWheelConstraintGroupState*& wheelConstraintGroups) override
    {
        axleDescription = &mBaseParams.axleDescription;
        tankDriveTransmissionCommands = &mTankDriveTransmissionCommandState;
        wheelParams.setData(mBaseParams.wheelParams);
        differentialParams = &mEngineDriveParams.tankDifferentialParams;
        differentialState = &mEngineDriveState.differentialState;
        wheelConstraintGroups = &mEngineDriveState.wheelConstraintGroupState;
    }

    void getDataForEngineDriveActuationStateComponent(
        const PxVehicleAxleDescription*& axleDescription,
        const PxVehicleGearboxParams*& gearboxParams,
        PxVehicleArrayData<const PxReal>& brakeResponseStates,
        const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
        const PxVehicleGearboxState*& gearboxState,
        const PxVehicleDifferentialState*& differentialState,
        const PxVehicleClutchCommandResponseState*& clutchResponseState,
        PxVehicleArrayData<PxVehicleWheelActuationState>& actuationStates) override
    {
        axleDescription = &mBaseParams.axleDescription;
        gearboxParams = &mEngineDriveParams.gearBoxParams;
        brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
        throttleResponseState = &mEngineDriveState.throttleCommandResponseState;
        gearboxState = &mEngineDriveState.gearboxState;
        differentialState = &mEngineDriveState.differentialState;
        clutchResponseState = &mEngineDriveState.clutchCommandResponseState;
        actuationStates.setData(mBaseState.actuationStates);
    }

    void getDataForEngineDrivetrainComponent(
        const PxVehicleAxleDescription*& axleDescription,
        PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
        const PxVehicleEngineParams*& engineParams,
        const PxVehicleClutchParams*& clutchParams,
        const PxVehicleGearboxParams*& gearboxParams,
        PxVehicleArrayData<const PxReal>& brakeResponseStates,
        PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
        PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
        const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
        const PxVehicleClutchCommandResponseState*& clutchResponseState,
        const PxVehicleDifferentialState*& differentialState,
        const PxVehicleWheelConstraintGroupState*& constraintGroupState,
        PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
        PxVehicleEngineState*& engineState,
        PxVehicleGearboxState*& gearboxState,
        PxVehicleClutchSlipState*& clutchState) override
    {
        axleDescription = &mBaseParams.axleDescription;
        wheelParams.setData(mBaseParams.wheelParams);
        engineParams = &mEngineDriveParams.engineParams;
        clutchParams = &mEngineDriveParams.clutchParams;
        gearboxParams = &mEngineDriveParams.gearBoxParams;
        brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
        actuationStates.setData(mBaseState.actuationStates);
        tireForces.setData(mBaseState.tireForces);
        throttleResponseState = &mEngineDriveState.throttleCommandResponseState;
        clutchResponseState = &mEngineDriveState.clutchCommandResponseState;
        differentialState = &mEngineDriveState.differentialState;
        constraintGroupState = Enum::eDIFFTYPE_TANKDRIVE == mDifferentialType ? &mEngineDriveState.wheelConstraintGroupState : nullptr;
        wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);
        engineState = &mEngineDriveState.engineState;
        gearboxState = &mEngineDriveState.gearboxState;
        clutchState = &mEngineDriveState.clutchState;
    }

    //Parameters and states of the vehicle's engine drivetrain.
    EngineDrivetrainParams mEngineDriveParams;
    EngineDrivetrainState mEngineDriveState;

    //The commands that will control the vehicle's transmission
    PxVehicleEngineDriveTransmissionCommandState mTransmissionCommandState;
    PxVehicleTankDriveTransmissionCommandState mTankDriveTransmissionCommandState;

    //The type of differential that will be used.
    //If eDIFFTYPE_TANKDRIVE is chosen then the vehicle's transmission
    //commands are stored in mTankDriveTransmissionCommandState.  
    //If eDIFFTYPE_FOURWHEELDRIVE or eDIFFTYPE_MULTIWHEELDRIVE is chosen
    //then the vehicle's transmission commands are stored in 
    //mTransmissionCommandState
    Enum mDifferentialType;
};

}
