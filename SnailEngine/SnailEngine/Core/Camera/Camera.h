#pragma once
#include <DirectXMath.h>

#include "Projections/Projection.h"
#include "Rendering/Buffers/D3D11Buffer.h"

#include "Core/Input/InputModule.h"
#include "Core/Math/Transform.h"
#include "Projections/OrthographicProjection.h"
#include "Projections/PerspectiveProjection.h"
#include "Util/SnailException.h"

namespace Snail
{
class UIElement;
class Entity;

class Camera
{
#ifdef _IMGUI_
    // Only currently useful for RenderImGui in projection being able to modify this class's fields
    friend Projection;
    friend PerspectiveProjection;
    friend OrthographicProjection;
#endif
public:
    enum CameraFollowMode
    {
        CAM_FREECAM,
        CAM_FIRST_PERSON,
        CAM_THIRD_PERSON
    };

    enum CameraProjectionType
    {
        CAM_PERSPECTIVE,
        CAM_ORTHOGRAPHIC
    };

    class InvalidCameraTypeException : public SnailException {};

protected:
    std::unique_ptr<Projection> projection;
    CameraProjectionType projectionType;

    Transform transform;
    float cameraSpeed = 10.0f;
    mutable struct TranformMatrixes
    {
        DirectX::XMFLOAT4X4 matViewProj{};
    } matrixes;
    D3D11Buffer matrixesBuffer;
    DirectX::XMFLOAT4X4 projectionMatrix{};

    CameraFollowMode followTargetMode = CAM_FREECAM;
    Entity* targetEntity = nullptr;

    // Height when in first person
    Vector3 firstPersonCameraOffset{ 0.0f, 0.4f, 0.7f };

    // Distance to the entity when in third person
    float cameraMinDistance = 2.0f;
    float targetOffset = 50.0f;
    float thirdPersonCameraDistance = 10.0f;
    float thirdPersonCameraYOffset = 3.0f;

    // Previous position of the target to calculate direction in third person
    Vector3 previousTargetPosition = Vector3::Zero;

    std::vector<std::string> overlayNames;
    std::map<std::string, const UIElement*> overlays;

    InputModule& input = InputModule::GetInstance();

    void RotateCamera(int dx, int dy);
    void UpdateFreeCam(float dt);
    void UpdateFirstPerson(float dt);
    void UpdateThirdPerson(float dt);

    Vector3 GetThirdPersonTargetPosition(const Vector3& targetPosition, const Vector3& movementDirection);

public:
    Camera(const Transform& initialTransform, float nearPlane, float farPlane, CameraProjectionType cameraProjection);
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    ~Camera() = default;

    void Resize(long width, long height) const;

    [[nodiscard]] Matrix GetViewMatrix() const;
    [[nodiscard]] Matrix GetProjectionMatrix() const;
    [[nodiscard]] Matrix GetProjectionMatrix(std::optional<float> nearPlane, std::optional<float> farPlane) const;
    [[nodiscard]] Matrix GetViewProjectionMatrix() const;

    bool IsPerspectiveCamera() const noexcept { return projectionType == CAM_PERSPECTIVE; }
    bool IsOrthographicCamera() const noexcept { return projectionType == CAM_ORTHOGRAPHIC; }
    bool IsFreeLookCamera() const noexcept { return targetEntity == nullptr && followTargetMode == CAM_FREECAM; }
    bool IsFirstPersonCamera() const noexcept { return targetEntity && followTargetMode == CAM_FIRST_PERSON; }
    bool IsThirdPersonCamera() const noexcept { return targetEntity && followTargetMode == CAM_THIRD_PERSON; }

    void SetCameraTarget(CameraFollowMode cameraFollowMode, Entity* target = nullptr);
    const Entity* GetTarget();

    const D3D11Buffer& GetTransformMatrixesBuffer();

    [[nodiscard]] Vector3 GetDirection() const;
    float GetFarPlane() const;
    float GetNearPlane() const;
    Projection* GetProjection() const;

    const Transform& GetTransform() const;
    Transform GetWorldTransform() const;
    void SetTransform(const Transform& cameraTransform);

    Quaternion GetLookAtQuat(const Vector3& dest) const;
    void Update(float dt);
    void UpdateControls(float dt);
    void DrawOverlays();

    void AddOverlay(const std::string& name, const UIElement* overlay);
    void RemoveOverlay(const std::string& name);

    bool RenderImGui();
};
}
