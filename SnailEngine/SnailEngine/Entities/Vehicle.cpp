#include "stdafx.h"

#include "Entities/Triggers/TriggerBox.h"
#include "vehicle.h"
#include "Core/WindowsEngine.h"
#include <algorithm>

namespace Snail
{
Vehicle::Vehicle(const Params& params)
    : Entity{params}
    , wheelMeshes(params.wheelMeshes)
    , wheelMeshOffsets(params.wheelMeshOffsets)
    , meshPhysicsOffset(params.meshPhysicsOffset)
{
    WindowsEngine::GetCamera()->SetCameraTarget(Camera::CAM_THIRD_PERSON, this);
    WindowsEngine::GetModule<GameManager>().SetVehicle(this);
}

Vehicle::~Vehicle()
{
    WindowsEngine::GetModule<GameManager>().SetVehicle(nullptr);
}

Vehicle::VehicleUserData::VehicleUserData(Entity* entity)
    : UserData{entity} {}

void Vehicle::VehicleUserData::OnTrigger(TriggerBox* triggerBox)
{
    LOGF("Car hit triggered \"{}\" trigger.", triggerBox->entityName);
}

bool Vehicle::IsUpsideDown()
{
    return transform.GetUpVector().Dot(Vector3::Up) < UPSIDE_DOWN_THRESHOLD;
}

void Vehicle::FlipCar()
{
    transform.rotation = Quaternion::CreateFromYawPitchRoll(transform.rotation.ToEuler().y, 0, 0);
    transform.position += Vector3::Up * 2;
    if (physicsVehicle) { physicsVehicle->SetTransform(transform); }
}

float Vehicle::GetForwardVelocity()
{
    return GetWorldTransform().GetForwardVector().Dot(physicsVehicle->GetLinearVelocity());
}

Vehicle::Params::Params()
{
    name = "vehicle";
    meshPhysicsOffset = Vector3::Zero;
}

void Vehicle::InitPhysics()
{
    Transform t = GetWorldTransform();

    userData = std::make_unique<VehicleUserData>(this);

    PhysicsVehicle* vehicle = new PhysicsVehicle(t);
    physicsVehicle = vehicle;
    vehicle->GetChassisShape()->userData = static_cast<void*>(userData.get());
    physicsObject.reset(vehicle);
}

void Vehicle::Update(const float dt) noexcept
{
    static auto& renderer = WindowsEngine::GetModule<RendererModule>();
    renderer.DrawLine({{GetWorldTransform().position}, {1, 0, 0}},
        {{GetWorldTransform().position + GetWorldTransform().GetForwardVector() * 5}, {1, 1, 0}});
    renderer.DrawLine({{GetWorldTransform().position}, {1, 0, 0}}, {{GetWorldTransform().position + physicsVehicle->GetLinearVelocity()}, {1, 0, 0}});

    static auto& cameraManager = WindowsEngine::GetModule<CameraManager>();
    static auto& gameManager = WindowsEngine::GetModule<GameManager>();
    if (cameraManager.GetCurrentCamera()->GetTarget() == this && gameManager.IsPlay())
    {
        static InputModule& input = InputModule::GetInstance();

        if (auto* controller = input.Controller.GetFirstActiveController(); controller && controller->IsNeutral())
        {
            const auto vals = controller->GetLeftJoystick();
            if (vals.x > 0.01f || vals.x < -0.01f)
            {
                physicsVehicle->Turn(vals.x); //steer
            }
            else
            {
                physicsVehicle->Turn(0);
            }

            const auto accelerateIntensity = controller->GetRightTrigger();
            const auto brakeIntensity = controller->GetLeftTrigger();

            if (accelerateIntensity > 0.01f)
            {
                physicsVehicle->Accelerate(accelerateIntensity);
            }
            else if (brakeIntensity > 0.01f)
            {
                if (GetForwardVelocity() < 0.01f) { physicsVehicle->Reverse(brakeIntensity); }
                else { physicsVehicle->Brake(brakeIntensity); }
            }
            else
            {
                physicsVehicle->Neutral();
            }

            if (controller->IsPressed(Controller::Buttons::B) && hasBoost)
            {
                hasBoost = false;
                WindowsEngine::GetModule<GameManager>().UseBoost();
                physicsVehicle->Boost(GetWorldTransform().GetForwardVector(), boostIntensity);
            }

            if (controller->IsPressed(Controller::Buttons::A) && IsUpsideDown()) { FlipCar(); } //flip

            if (controller->IsPressed(Controller::Buttons::Y)) //camera switch
            {
                Camera* cam = WindowsEngine::GetCamera();
                cam->SetCameraTarget(!cam->IsFirstPersonCamera() ? Camera::CAM_FIRST_PERSON : Camera::CAM_THIRD_PERSON, this);
            }
        }
        else
        {
            auto& state = input.Keyboard.GetState();

            const float intensity = 0.75f;
            const float turnRadius = 0.75f;

            if (state.IsKeyDown(Keyboard::D))
            {
                physicsVehicle->Turn(turnRadius);
            }
            else if (state.IsKeyDown(Keyboard::A))
            {
                physicsVehicle->Turn(-turnRadius);
            }
            else
            {
                physicsVehicle->Turn(0.0f);
            }

            if (state.IsKeyDown(Keyboard::W))
            {
                physicsVehicle->Accelerate(intensity);
            }
            else if (state.IsKeyDown(Keyboard::S))
            {
                if (GetForwardVelocity() < 0.1f)
                {
                    physicsVehicle->Reverse(intensity);
                }
                else
                {
                    physicsVehicle->Brake(intensity);
                }
            }
            else
            {
                physicsVehicle->Neutral();
            }

            if (state.IsKeyPressed(Keyboard::F))
            {
                Camera* cam = WindowsEngine::GetCamera();
                cam->SetCameraTarget(!cam->IsFirstPersonCamera() ? Camera::CAM_FIRST_PERSON : Camera::CAM_THIRD_PERSON, this);
            }

            if (state.IsKeyPressed(Keyboard::E) && hasBoost)
            {
                hasBoost = false;
                WindowsEngine::GetModule<GameManager>().UseBoost();
                physicsVehicle->Boost(GetWorldTransform().GetForwardVector(), boostIntensity);
            }

            if (state.IsKeyPressed(Keyboard::R) && IsUpsideDown())
            {
                FlipCar();
            }
        }
    }

    if (physicsVehicle)
    {
        if (isOnGrass)
            physicsVehicle->SlowToSpeed(grassSpeedThreshold);

        isOnGrass = false;
        dirtyFlags.set();
        physicsVehicle->Update(dt);
        physicsVehicle->UpdateTransform(transform, meshPhysicsOffset);
        physicsVehicle->UpdateTransformWheels(wheelTransforms, wheelMeshOffsets);
    }
}

void Vehicle::CollectBoost()
{
    hasBoost = true;
}

bool Vehicle::HasBoost()
{
    return hasBoost;
}

void Vehicle::SetOnGrass(const bool onGrass)
{
    isOnGrass = onGrass;
}

void Vehicle::SetTransform(const Transform& t)
{
    dirtyFlags.set();

    if (!physicsVehicle)
    {
        transform = t;
        return;
    }

    if (physicsVehicle->SetTransform(t))
    {
        transform = t;
        transform.position += meshPhysicsOffset;
    }
}

void Vehicle::Draw(DrawContext& ctx)
{
    Entity::Draw(ctx);

    for (auto i = 0; i < 4; i++)
    {
        if (!wheelMeshes[i] || (shouldFrustumCull && ctx.ShouldBeCulled(GetBoundingBox())))
            return;
        wheelMeshes[i]->SubscribeInstance(wheelTransforms[i].GetTransformationMatrix());
    }
}

void Vehicle::SetPosition(const Vector3& pos)
{
    dirtyFlags.set();

    if (!physicsVehicle)
    {
        transform.position = pos;
        return;
    }

    Transform newTransform = transform;
    newTransform.position = pos;
    if (physicsVehicle->SetTransform(newTransform)) { transform.position = pos; }
}

void Vehicle::SetRotation(const Vector3& euler)
{
    dirtyFlags.set();
    transform.rotation = Quaternion::CreateFromYawPitchRoll(euler);
    if (physicsVehicle) { physicsVehicle->SetTransform(transform); }
}

void Vehicle::RenderImGui(const int idNumber)
{
    UNREFERENCED_PARAMETER(idNumber);
#ifdef _IMGUI_
    ImGui::DragFloat3("Physics/Mesh Offset", &meshPhysicsOffset.x);
    ImGui::DragFloat("Grass breaking factor", &grassBreakingFactor, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Grass breaking threshold", &grassSpeedThreshold, 0.1f, 0.0f, 100000.0f);
    Entity::RenderImGui(idNumber);
#endif
}

std::string Vehicle::GetJsonType()
{
    return "vehicle";
}
}
