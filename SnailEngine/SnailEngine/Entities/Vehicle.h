#pragma once

#include "Entity.h"
#include "Core/Physics/PhysicsVehicle.h"
#include "Core/Math/SimpleMath.h"
#include "Rendering/Effects/PostProcessing/VignetteEffect.h"
#include <array>

namespace Snail {

    class Vehicle : public Entity {

        struct VehicleUserData final : UserData
        {
            VehicleUserData(Entity* entity);
            void OnTrigger(TriggerBox*) override;
        };

        std::array<BaseMesh*, 4> wheelMeshes;
        std::array<Transform, 4> wheelTransforms;
        std::array<Vector3, 4> wheelMeshOffsets;

        PhysicsVehicle* physicsVehicle = nullptr;
        Vector3 meshPhysicsOffset;
        bool hasBoost = false;
        bool isOnGrass = false;
        float grassBreakingFactor = 1;
        float grassSpeedThreshold = 10;
        float boostIntensity = 40000;

        static constexpr float UPSIDE_DOWN_THRESHOLD = 0.3f;
        bool IsUpsideDown();
        void FlipCar();
        float GetForwardVelocity();
    public:
        struct Params : Entity::Params
        {
            Vector3 meshPhysicsOffset;

            std::array<BaseMesh*, 4> wheelMeshes;
            std::array<Transform, 4> wheelTransforms;
            std::array<Vector3, 4> wheelMeshOffsets;

            Params();
        };

        Vehicle(const Params& params = {});
        ~Vehicle() override;

        void InitPhysics() override;
        void Update(float) noexcept override;
        void Draw(DrawContext& ctx) override;
        void CollectBoost();
        bool HasBoost();

        void SetOnGrass(bool onGrass);

        void SetTransform(const Transform& t) override;
        void SetPosition(const Vector3& pos) override;
        void SetRotation(const Vector3& euler) override;

        PhysicsVehicle* GetPhysicsVehicle() const noexcept { return physicsVehicle; }

        void RenderImGui(int idNumber) override;

        std::string GetJsonType() override;
    };

};
