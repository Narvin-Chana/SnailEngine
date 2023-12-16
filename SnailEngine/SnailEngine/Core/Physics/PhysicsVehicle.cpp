#include "stdafx.h"

#include "PhysicsVehicle.h"

#include "Core/WindowsEngine.h"

#include "Util/PhysX/BaseSerialization.h"
#include "Util/PhysX/DirectDrivetrainSerialization.h"
#include "Util/PhysX/EngineDrivetrainSerialization.h"

using namespace physx;

namespace Snail
{
PhysicsVehicle::PhysicsVehicle(const Transform& initialTransform)
{
    std::unique_lock lock{PhysxMutex};

    static PhysicsModule& physicsMod = WindowsEngine::GetModule<PhysicsModule>();

    readBaseParamsFromJsonFile(DefaultParamsDirectory, DefaultVehicleBaseParamsFile, engineDriveVehicle.mBaseParams);
    SetPhysXIntegrationParams(engineDriveVehicle.mBaseParams.axleDescription,
        physicsMod.materialFrictions,
        physicsMod.nbMaterialFrictions,
        physicsMod.defaultMaterialFriction,
        engineDriveVehicle.mPhysXParams);
    readEngineDrivetrainParamsFromJsonFile(DefaultParamsDirectory, DefaultVehicleEngineDriveParams, engineDriveVehicle.mEngineDriveParams);

    //Set the states to default.
    if (!engineDriveVehicle.Initialize(*physicsMod.physics,
        PxCookingParams(PxTolerancesScale()),
        *physicsMod.defaultMaterial,
        EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE))
    {
        LOG(Logger::FATAL, "Unable to instantiate vehicle physics!");
    }

    engineDriveVehicle.SetUpActor(*physicsMod.scene, initialTransform, "Vehicle");

    engineDriveVehicle.mCommandState.nbBrakes = 1;
    engineDriveVehicle.mCommandState.brakes[0] = 0;
    engineDriveVehicle.mEngineDriveState.gearboxState.currentGear = engineDriveVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
    engineDriveVehicle.mEngineDriveState.gearboxState.targetGear = engineDriveVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
    engineDriveVehicle.mTransmissionCommandState.targetGear = static_cast<PxU32>(GearState::AUTOMATIC);

    // Set up the simulation context.
    // SnailEngine uses:
    //      a) z as the longitudinal axis
    //      b) x as the lateral axis
    //      c) y as the vertical axis.
    //      d) metres  as the lengthscale.
    gVehicleSimulationContext.setToDefault();
    gVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
    gVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
    gVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
    gVehicleSimulationContext.scale.scale = 1.0f;
    gVehicleSimulationContext.gravity = physicsMod.scene->getGravity();
    gVehicleSimulationContext.physxScene = physicsMod.scene.get();
    gVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

}

bool PhysicsVehicle::SetTransform(const Transform& transform)
{
    engineDriveVehicle.GetRigidBody()->setGlobalPose(transform);
    return true;
}

void PhysicsVehicle::UpdateTransform(Transform& transform, const Vector3& meshOffset)
{
    PxTransform pPos = engineDriveVehicle.GetRigidBody()->getGlobalPose();
    pPos.p = pPos.transform(PxVec3(meshOffset.x, meshOffset.y, meshOffset.z));
    transform.UpdateFromPhysics(pPos);
}

void PhysicsVehicle::UpdateTransformWheels(std::array<Transform, 4>& transforms, const std::array<Vector3, 4>& meshOffsets)
{
    const std::array<PxShape*, 4> wheelShapes = engineDriveVehicle.GetWheelShape();

    for (int i = 0; i < 4; ++i) {
        PxTransform pPos = engineDriveVehicle.GetRigidBody()->getGlobalPose();
        pPos.p = pPos.transform(PxVec3(meshOffsets[i].x, meshOffsets[i].y, meshOffsets[i].z));
        PxTransform wheelPose = pPos.transform( wheelShapes[i]->getLocalPose());
        transforms[i].UpdateFromPhysics(wheelPose);
    }
}


Vector3 PhysicsVehicle::GetLinearVelocity()
{
    PxVec3 globalLinearVelocity = engineDriveVehicle.GetRigidBody()->getLinearVelocity();

    return {globalLinearVelocity.x, globalLinearVelocity.y, globalLinearVelocity.z};
}

void PhysicsVehicle::Accelerate(const float intensity)
{
    LOG(Logger::INFO, "intensite voiture, ", intensity);
    LOG(Logger::INFO, "boite vitesse voiture, ", engineDriveVehicle.mEngineDriveState.gearboxState.currentGear);
    LOG(Logger::INFO, "boite vitesse voiture vqleur, ", engineDriveVehicle.mEngineDriveParams.gearBoxParams.ratios[2]);
    LOG(Logger::INFO, "boite vitesse target voiture, ", engineDriveVehicle.mTransmissionCommandState.targetGear);
    LOG(Logger::INFO, "frein voiture, ", engineDriveVehicle.mCommandState.brakes[0]);
    LOG(Logger::INFO, "nb frein voiture, ", engineDriveVehicle.mCommandState.nbBrakes);
    engineDriveVehicle.mCommandState.brakes[0] = 0;
    engineDriveVehicle.mCommandState.throttle = intensity;
    engineDriveVehicle.mTransmissionCommandState.targetGear = static_cast<PxU32>(GearState::AUTOMATIC);
}

void PhysicsVehicle::Reverse(const float intensity)
{
    engineDriveVehicle.mCommandState.brakes[0] = 0;
    engineDriveVehicle.mCommandState.throttle = intensity;
    engineDriveVehicle.mTransmissionCommandState.targetGear = static_cast<PxU32>(GearState::REVERSE);
}

void PhysicsVehicle::Turn(const float intensity)
{
    engineDriveVehicle.mCommandState.steer = intensity;
}

void PhysicsVehicle::Brake(const float intensity)
{
    engineDriveVehicle.mCommandState.brakes[0] = intensity;
    engineDriveVehicle.mCommandState.throttle = 0;
}

void PhysicsVehicle::SlowToSpeed(float)
{
    // TODO: Implement this in the future
    //if (GetLinearVelocity().Length() > targetSpeed)
    //{
    //    // find a way to slow down faster without loosing control here
    //    directDriveVehicle.mCommandState.throttle = std::min(directDriveVehicle.mCommandState.throttle, 0.0f);
    //}
}

void PhysicsVehicle::Neutral()
{
    engineDriveVehicle.mTransmissionCommandState.targetGear = static_cast<PxU32>(GearState::NEUTRAL);
    //Brake(0.3f);
}

void PhysicsVehicle::Update(const float dt)
{
    // Apply substepping at low forward speed to improve simulation fidelity
    const PxVec3 linVel = engineDriveVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
    const PxVec3 forwardDir = engineDriveVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
    const PxReal forwardSpeed = linVel.dot(forwardDir);
    const PxU8 nbSubsteps = forwardSpeed < 5.0f ? 3 : 1;
    engineDriveVehicle.mComponentSequence.setSubsteps(engineDriveVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);

    engineDriveVehicle.Step(dt, gVehicleSimulationContext);
}

PhysicsVehicle::~PhysicsVehicle()
{
    engineDriveVehicle.Destroy();
}

PxShape* PhysicsVehicle::GetChassisShape()
{
    return engineDriveVehicle.GetMainChassisShape();
}

void PhysicsVehicle::Boost(const Vector3& direction, const float intensity)
{
    engineDriveVehicle.GetRigidBody()->addForce({direction.x * intensity, direction.y * intensity, direction.z * intensity}, PxForceMode::eIMPULSE);
}

float PhysicsVehicle::GetSpeed() const
{
    const PxRigidBody* driveBody = engineDriveVehicle.GetRigidBody();
    return driveBody ? driveBody->getLinearVelocity().magnitude() : 0.0f;
}

void PhysicsVehicle::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text("Physics Type: Vehicle");
#endif
}

}
