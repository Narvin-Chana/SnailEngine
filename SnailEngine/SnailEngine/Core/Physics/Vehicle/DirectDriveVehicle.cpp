#include "stdafx.h"
#include "DirectDriveVehicle.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

DirectDrivetrainParams DirectDrivetrainParams::TransformAndScale(
    const PxVehicleFrame& srcFrame,
    const PxVehicleFrame& trgFrame,
    const PxVehicleScale& srcScale,
    const PxVehicleScale& trgScale) const
{
    DirectDrivetrainParams r = *this;
    r.directDriveThrottleResponseParams = directDriveThrottleResponseParams.transformAndScale(srcFrame, trgFrame, srcScale, trgScale);
    return r;
}

bool DirectDriveVehicle::Initialize(PxPhysics& physics, const PxCookingParams& params, PxMaterial& defaultMaterial, const bool addPhysXBeginEndComponents)
{
    mTransmissionCommandState.setToDefault();

    if (!PhysXActorVehicle::Initialize(physics, params, defaultMaterial))
        return false;

    if (!mDirectDriveParams.IsValid(mBaseParams.axleDescription))
        return false;

    //Set the drivetrain state to default.
    mDirectDriveState.SetToDefault();

    //Add all the components in sequence that will simulate a vehicle with a direct drive drivetrain.
    InitComponentSequence(addPhysXBeginEndComponents);

    return true;
}

void DirectDriveVehicle::Destroy()
{
    PhysXActorVehicle::Destroy();
}

void DirectDriveVehicle::InitComponentSequence(const bool addPhysXBeginEndComponents)
{
    //Wake up the associated PxRigidBody if it is asleep and the vehicle commands signal an
    //intent to change state.
    //Read from the physx actor and write the state (position, velocity etc) to the vehicle.
    if (addPhysXBeginEndComponents)
        mComponentSequence.add(static_cast<PxVehiclePhysXActorBeginComponent*>(this));

    //Read the input commands (throttle, brake etc) and forward them as torques and angles to the wheels on each axle.
    mComponentSequence.add(static_cast<PxVehicleDirectDriveCommandResponseComponent*>(this));

    //Work out which wheels have a non-zero drive torque and non-zero brake torque.
    //This is used to determine if any tire is to enter the "sticky" regime that will bring the 
    //vehicle to rest.
    mComponentSequence.add(static_cast<PxVehicleDirectDriveActuationStateComponent*>(this));

    //Perform a scene query against the physx scene to determine the plane and friction under each wheel.
    mComponentSequence.add(static_cast<PxVehiclePhysXRoadGeometrySceneQueryComponent*>(this));

    //Start a substep group that can be ticked multiple times per update.
    //In this example, we update the suspensions, tires and wheels 3 times without recalculating 
    //the plane underneath the wheel.  This is useful for stability at low forward speeds and is
    //computationally cheaper than simulating the whole pipeline at a smaller timestep.
    mComponentSequenceSubstepGroupHandle = mComponentSequence.beginSubstepGroup(3);

    //Update the suspension compression given the plane under each wheel.
    //Update the kinematic compliance from the compression state of each suspension.
    //Convert suspension state to suspension force and torque.
    mComponentSequence.add(static_cast<PxVehicleSuspensionComponent*>(this));

    //Compute the load on the tire, the friction experienced by the tire 
    //and the lateral/longitudinal slip angles.
    //Convert load/friction/slip to tire force and torque.
    //If the vehicle is to come rest then compute the "sticky" velocity constraints to apply to the
    //vehicle.
    mComponentSequence.add(static_cast<PxVehicleTireComponent*>(this));

    //Apply any velocity constraints to a data buffer that will be consumed by the physx scene
    //during the next physx scene update.
    mComponentSequence.add(static_cast<PxVehiclePhysXConstraintComponent*>(this));

    //Apply the tire force, brake force and drive force to each wheel and
    //forward integrate the rotation speed of each wheel.
    mComponentSequence.add(static_cast<PxVehicleDirectDrivetrainComponent*>(this));

    //Apply the suspension and tire forces to the vehicle's rigid body and forward 
    //integrate the state of the rigid body.
    mComponentSequence.add(static_cast<PxVehicleRigidBodyComponent*>(this));

    //Mark the end of the substep group.
    mComponentSequence.endSubstepGroup();

    //Update the rotation angle of the wheel by forwarding integrating the rotational
    //speed of each wheel.
    //Compute the local pose of the wheel in the rigid body frame after accounting 
    //suspension compression and compliance.
    mComponentSequence.add(static_cast<PxVehicleWheelComponent*>(this));

    //Write the local poses of each wheel to the corresponding shapes on the physx actor.
    //Write the momentum change applied to the vehicle's rigid body to the physx actor.
    //The physx scene can now try to apply that change to the physx actor.
    //The physx scene will account for collisions and constraints to be applied to the vehicle 
    //that occur by applying the change.
    if (addPhysXBeginEndComponents)
        mComponentSequence.add(static_cast<PxVehiclePhysXActorEndComponent*>(this));
}

PxRigidBody* DirectDriveVehicle::GetRigidBody()
{
    return mPhysXState.physxActor.rigidBody;
}

PxRigidBody* DirectDriveVehicle::GetRigidBody() const
{
    return mPhysXState.physxActor.rigidBody;
}

PxShape* DirectDriveVehicle::GetMainChassisShape()
{
    assert(mPhysXState.physxActor.rigidBody->getNbShapes() == 5);
    PxShape* shapes[5]{};
    mPhysXState.physxActor.rigidBody->getShapes(shapes, 5);
    
    return shapes[0];
}

void DirectDriveVehicle::getDataForDirectDriveCommandResponseComponent(
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
    PxVehicleArrayData<PxReal>& steerResponseStates)
{
    axleDescription = &mBaseParams.axleDescription;
    brakeResponseParams.setDataAndCount(mBaseParams.brakeResponseParams,
        sizeof(mBaseParams.brakeResponseParams) / sizeof(PxVehicleBrakeCommandResponseParams));
    throttleResponseParams = &mDirectDriveParams.directDriveThrottleResponseParams;
    steerResponseParams = &mBaseParams.steerResponseParams;
    ackermannParams.setDataAndCount(mBaseParams.ackermannParams, sizeof(mBaseParams.ackermannParams) / sizeof(PxVehicleAckermannParams));
    commands = &mCommandState;
    transmissionCommands = &mTransmissionCommandState;
    rigidBodyState = &mBaseState.rigidBodyState;
    brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
    throttleResponseStates.setData(mDirectDriveState.directDriveThrottleResponseStates);
    steerResponseStates.setData(mBaseState.steerCommandResponseStates);
}

void DirectDriveVehicle::getDataForDirectDriveActuationStateComponent(
    const PxVehicleAxleDescription*& axleDescription,
    PxVehicleArrayData<const PxReal>& brakeResponseStates,
    PxVehicleArrayData<const PxReal>& throttleResponseStates,
    PxVehicleArrayData<PxVehicleWheelActuationState>& actuationStates)
{
    axleDescription = &mBaseParams.axleDescription;
    brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
    throttleResponseStates.setData(mDirectDriveState.directDriveThrottleResponseStates);
    actuationStates.setData(mBaseState.actuationStates);
}

void DirectDriveVehicle::getDataForDirectDrivetrainComponent(
    const PxVehicleAxleDescription*& axleDescription,
    PxVehicleArrayData<const PxReal>& brakeResponseStates,
    PxVehicleArrayData<const PxReal>& throttleResponseStates,
    PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
    PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
    PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
    PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates)
{
    axleDescription = &mBaseParams.axleDescription;
    brakeResponseStates.setData(mBaseState.brakeCommandResponseStates);
    throttleResponseStates.setData(mDirectDriveState.directDriveThrottleResponseStates);
    wheelParams.setData(mBaseParams.wheelParams);
    actuationStates.setData(mBaseState.actuationStates);
    tireForces.setData(mBaseState.tireForces);
    wheelRigidBody1dStates.setData(mBaseState.wheelRigidBody1dStates);
}

}
