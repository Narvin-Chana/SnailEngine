#pragma once

#include "PhysicsObject.h"
#include "Core/Math/Transform.h"
#include "Vehicle/DirectDriveVehicle.h"
#include "Vehicle/EngineDriveVehicle.h"
#include <array>

namespace physx
{
class PxShape;
}

namespace Snail
{

static constexpr const char* DefaultParamsDirectory = "Resources/VehicleData";
static constexpr const char* DefaultVehicleBaseParamsFile = "Base.json";
static constexpr const char* DefaultVehicleEngineDriveParams = "EngineDrive.json";

class PhysicsVehicle : public PhysicsObject
{
    //Vehicle simulation needs a simulation context
    //to store global parameters of the simulation such as 
    //gravitational acceleration.
    PxVehiclePhysXSimulationContext gVehicleSimulationContext;
    inline static physx::PxVec3 force{0.0f, 0.0f, -20000.0f};

    enum class GearState : PxU32
    {
        REVERSE = 0,
        NEUTRAL = 1,
        AUTOMATIC = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR
    };

protected:
    EngineDriveVehicle engineDriveVehicle;

public:
    PhysicsVehicle(const Transform& initialTransform);
    ~PhysicsVehicle() override;

    PxShape* GetChassisShape();

    void SetShape(physx::PxShape*) override
    {
        LOG(Logger::ERROR, "Cannot SetShape of a vehicle entity.");
    }

    bool SetTransform(const Transform& transform) override;
    void UpdateTransform(Transform& transform, const Vector3& meshOffset);
    void UpdateTransformWheels(std::array<Transform, 4>& transforms, const std::array<Vector3, 4>& meshOffsets);

    Vector3 GetLinearVelocity();

    void Accelerate(float intensity);
    void Reverse(float intensity);
    /**
     * \brief Set turning radius.
     * \param intensity Intensity of the turn (from -1 to 1)
     */
    void Turn(float intensity);
    void Brake(float intensity);
    void SlowToSpeed(float targetSpeed);
    void Neutral();

    float GetSpeed() const;
    void Boost(const Vector3& direction, float intensity);

    void Update(float dt);

    void RenderImGui() override;
};
}
