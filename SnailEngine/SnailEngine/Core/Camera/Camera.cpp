#include "stdafx.h"
#include "Camera.h"

#include <string>

#include "Projections/PerspectiveProjection.h"
#include "Core/Mesh/Mesh.h"
#include "Core/Input/Controller.h"
#include "Core/WindowsEngine.h"
#include "Entities/Entity.h"

#include "Rendering/UI/UIElement.h"

namespace Snail
{

void Camera::RotateCamera(const int dx, const int dy)
{
    // To have a full 360 rotation in each way
    //const Quaternion rotx = Quaternion::CreateFromAxisAngle(transform.GetUpVector(), dx / 1000.0f);

    // To act like a fps camera
    const Quaternion rotx = Quaternion::CreateFromAxisAngle(Vector3::UnitY, dx / 1000.0f);
    const Quaternion roty = Quaternion::CreateFromAxisAngle(transform.GetRightVector(), dy / 1000.0f);
    transform.rotation *= roty * rotx;
}

Camera::Camera(const Transform& initialTransform, const float nearPlane, const float farPlane, const CameraProjectionType cameraProjection)
    : projectionType(cameraProjection)
    , transform{initialTransform}
    , matrixesBuffer{D3D11_BIND_CONSTANT_BUFFER}
{
    switch (projectionType)
    {
    case CAM_PERSPECTIVE:
        projection = std::make_unique<PerspectiveProjection>();
        break;
    case CAM_ORTHOGRAPHIC:
        projection = std::make_unique<OrthographicProjection>();
        break;
    default:
        {
            LOG(Logger::ERROR, "Invalid projection type when trying to initialise camera.");
            throw InvalidCameraTypeException();
        }
    }

    projection->nearPlane = nearPlane;
    projection->farPlane = farPlane;

    static WindowsEngine& engine = WindowsEngine::GetInstance();
    const auto& [screenWidth, screenHeight] = engine.GetRenderDevice()->GetResolutionSize();
    projection->SetupProjectionMatrix(screenWidth, screenHeight);
}

void Camera::Resize(const long width, const long height) const
{
    projection->SetupProjectionMatrix(width, height);
}

Vector3 Camera::GetDirection() const { return transform.GetForwardVector(); }

float Camera::GetFarPlane() const { return projection->farPlane; }

float Camera::GetNearPlane() const { return projection->nearPlane; }

Projection* Camera::GetProjection() const
{
    return projection.get();
}

const Transform& Camera::GetTransform() const { return transform; }

Transform Camera::GetWorldTransform() const
{
    return transform;
}

void Camera::SetTransform(const Transform& cameraTransform)
{
    transform = cameraTransform;
}

Quaternion Camera::GetLookAtQuat(const Vector3& dest) const
{
    const Matrix mat = Matrix::CreateLookAt(transform.position, dest, Vector3::Up);
    Quaternion quat = Quaternion::CreateFromRotationMatrix(mat);
    quat.Inverse(quat);
    return quat;
}

void Camera::Update(float){}

void Camera::UpdateControls(const float dt)
{
#ifndef _DEBUG
    if (WindowsEngine::GetInstance().isMainMenuLoaded) return;
#endif // 
    switch (followTargetMode)
    {
    case CAM_FREECAM:
        UpdateFreeCam(dt);
        break;
    case CAM_FIRST_PERSON:
        UpdateFirstPerson(dt);
        break;
    case CAM_THIRD_PERSON:
        UpdateThirdPerson(dt);
        break;
    default: ;
    }
}

void Camera::DrawOverlays()
{
    for (const std::string& overlay : overlayNames)
        overlays[overlay]->Draw();
}

void Camera::AddOverlay(const std::string& name, const UIElement* overlay)
{
    overlayNames.push_back(name);
    overlays[name] = overlay;

    std::ranges::sort(overlayNames, [&](const auto& a, const auto& b) {
        return overlays[a]->zOrder < overlays[b]->zOrder;
    });
}

void Camera::RemoveOverlay(const std::string& name)
{
    std::erase(overlayNames, name);
    overlays.erase(name);
}

void Camera::SetCameraTarget(const CameraFollowMode cameraFollowMode, Entity* target)
{
    followTargetMode = cameraFollowMode;
    previousTargetPosition = Vector3::Zero;
    thirdPersonCameraDistance = 10.0f;

    switch (cameraFollowMode)
    {
    case CAM_FIRST_PERSON:
        transform = {};
        targetEntity = target;
        
        break;
    case CAM_THIRD_PERSON:
        {
            targetEntity = target;
            const auto worldTrans = targetEntity->GetWorldTransform();
            transform.position = GetThirdPersonTargetPosition(worldTrans.position, worldTrans.GetForwardVector());
            transform.rotation = GetLookAtQuat(worldTrans.position + worldTrans.GetForwardVector() * targetOffset);
        }
        break;
    case CAM_FREECAM:
        targetEntity = nullptr;
        break;
    default:
        throw InvalidCameraTypeException();
    }
}

const Entity* Camera::GetTarget()
{
    return targetEntity;
}

const D3D11Buffer& Camera::GetTransformMatrixesBuffer()
{
    const Matrix viewProjMat = GetViewProjectionMatrix();

    matrixesBuffer.UpdateData(viewProjMat.Transpose());
    return matrixesBuffer;
}

Matrix Camera::GetViewMatrix() const
{
    return Matrix::CreateLookAt(GetTransform().position, GetTransform().position + GetTransform().GetForwardVector(), GetTransform().GetUpVector());
}

Matrix Camera::GetProjectionMatrix() const { return projection->GetProjectionMatrix(); }

Matrix Camera::GetViewProjectionMatrix() const { return GetViewMatrix() * GetProjectionMatrix(); }

Matrix Camera::GetProjectionMatrix(const std::optional<float> nearPlane, const std::optional<float> farPlane) const
{
    return projection->GetProjectionMatrix(nearPlane.value_or(projection->nearPlane), farPlane.value_or(projection->farPlane));
}

void Camera::UpdateFreeCam(const float dt)
{
    const auto& [up, right, forward] = transform.GetAxis();

    Vector3 movement{};

    if (auto* controller = input.Controller.GetFirstActiveController(); controller && controller->IsNeutral())
    {
        const auto vals = controller->GetLeftJoystick();
        movement += forward * vals.y * cameraSpeed * dt;
        movement += right * vals.x * cameraSpeed * dt;
        movement -= up * controller->GetLeftTrigger() * cameraSpeed * dt;
        movement += up * controller->GetRightTrigger() * cameraSpeed * dt;

        if (controller->IsDown(Controller::Buttons::RB)) { cameraSpeed = std::max(cameraSpeed + 1, 0.0f); }

        if (controller->IsDown(Controller::Buttons::LB)) { cameraSpeed = std::max(cameraSpeed - 1, 0.0f); }

        const auto rightJoystick = controller->GetRightJoystick();
        RotateCamera(static_cast<int>(rightJoystick.x * 255 * 0.1f), -static_cast<int>(rightJoystick.y * 255 * 0.1f));
    }

    const auto rightClick = input.Mouse.GetButtonStateFlags(&Mouse::MouseButtonState::rightButton);
    if (input.Mouse.IsStatePressed(rightClick))
    {
        input.Mouse.SetMode(Mouse::MODE_RELATIVE);
        input.Mouse.SetVisible(false);
        input.Mouse.SetLock(true);
    }
    else if (input.Mouse.IsStateReleased(rightClick))
    {
        input.Mouse.SetMode(Mouse::MODE_ABSOLUTE);
        input.Mouse.SetVisible(true);
        input.Mouse.SetLock(false);
    }

    if (input.Mouse.IsStateDown(rightClick))
    {
        const auto mouseState = input.Mouse.GetMouseState();
        RotateCamera(mouseState.x, mouseState.y);
        if (mouseState.scrollWheelDelta != 0) { cameraSpeed = std::max(cameraSpeed + mouseState.scrollWheelDelta / 100, 0.0f); }

        auto& kbState = input.Keyboard.GetState();
        if (projectionType == CAM_PERSPECTIVE)
        {
            if (kbState.IsKeyDown(Keyboard::D))
                movement += right * cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::A))
                movement -= right * cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::W))
                movement += forward * cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::S))
                movement -= forward * cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::E))
                movement += up * cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::Q))
                movement -= up * cameraSpeed * dt;
        }
        else if (projectionType == CAM_ORTHOGRAPHIC)
        {
            OrthographicProjection* orthographicProjection = dynamic_cast<OrthographicProjection*>(projection.get());

            assert(orthographicProjection);

            if (kbState.IsKeyDown(Keyboard::W))
                orthographicProjection->zoom += cameraSpeed * dt;
            if (kbState.IsKeyDown(Keyboard::S))
                orthographicProjection->zoom = std::max(orthographicProjection->zoom - cameraSpeed * dt, 1.0f);

            orthographicProjection->SetupProjectionMatrix(orthographicProjection->width, orthographicProjection->height);
        }
    }
    transform.position += movement;
}

void Camera::UpdateFirstPerson(const float)
{
    assert(targetEntity);
    // Could add in an optional offset here if needed
    transform = targetEntity->GetWorldTransform();
    const auto targetWorldTransform = targetEntity->GetWorldTransform();
    auto offset = Vector3::Transform(firstPersonCameraOffset, targetWorldTransform.GetTransformationMatrix());
    transform.position = offset;
}

void Camera::UpdateThirdPerson(const float dt)
{
    
    assert(targetEntity);

    const auto targetWorldTransform = targetEntity->GetWorldTransform();

    // Use the object's forward vector as the default direction if it's not moving
    const Vector3 movementDirection = targetWorldTransform.GetForwardVector();
    const Vector3 targetPosition = targetWorldTransform.position;

    transform.rotation = Quaternion::Slerp(transform.rotation, GetLookAtQuat(targetPosition + movementDirection * targetOffset), dt * cameraSpeed);

    // Calculate the new camera position with the adjusted movement direction
    transform.position = Vector3::Lerp(transform.position, GetThirdPersonTargetPosition(targetPosition, movementDirection), dt * cameraSpeed);

    // Save the current target position for the next frame
    previousTargetPosition = targetPosition;
}
Vector3 Camera::GetThirdPersonTargetPosition(const Vector3& targetPosition, const Vector3& movementDirection)
{
    static auto& physicsModule = WindowsEngine::GetModule<PhysicsModule>();
    // Calculate the new position based on the target's position and movement direction
    Vector3 nextPosition = targetPosition + Vector3(0, thirdPersonCameraYOffset, 0) - movementDirection * thirdPersonCameraDistance;

    // Get camera closer to car when wall is behind
    Vector3 direction = nextPosition - targetPosition;
    const float dirDistance = direction.Length();
    direction.Normalize(direction);
    if (const float dist = physicsModule.RaycastDistance(targetPosition, direction); dist < dirDistance)
    {
        nextPosition = targetPosition + direction * std::max(dist, cameraMinDistance);
    }

    return nextPosition;
}

bool Camera::RenderImGui()
{
#ifdef _IMGUI_
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Change Current Camera: ");

        static int eView = 0;
        const int eViewCopy = eView;

        static int eControl = 0;
        const int eControlCopy = eControl;

        static int eBoth = 0;
        const int eBothCopy = eBoth;

        // Consider that a maximum of 99 cameras exist to avoid constantly resizing
        static std::array<bool, 99> drawFrustum{false};
        static CameraManager& cameraManager = WindowsEngine::GetModule<CameraManager>();
        Camera* currentCamera = cameraManager.GetCamera(0);
        for (int i = 0; currentCamera; currentCamera = cameraManager.GetCamera(++i))
        {
            std::string cameraType = currentCamera->IsPerspectiveCamera() ? "Perspective" : "Orthographic";
            ImGui::Text(("Camera " + std::to_string(i) + " (" + cameraType + ")").c_str());

            switch (currentCamera->followTargetMode)
            {
            case CAM_FIRST_PERSON:
                {
                    ImGui::DragFloat3(("Offset##FPS Offset" + std::to_string(i)).c_str(), &currentCamera->firstPersonCameraOffset.x);
                }
                break;
            case CAM_THIRD_PERSON:
                {
                    ImGui::DragFloat(("Y offset##TPS YOffset" + std::to_string(i)).c_str(), &currentCamera->thirdPersonCameraYOffset);
                    ImGui::DragFloat(("Target offset##TPS TargetOffset" + std::to_string(i)).c_str(), &currentCamera->targetOffset);
                    ImGui::DragFloat(("Distance##Distance" + std::to_string(i)).c_str(), &currentCamera->thirdPersonCameraDistance);
                    ImGui::DragFloat(("Min Distance##MinDistance" + std::to_string(i)).c_str(), &currentCamera->cameraMinDistance);
                }
                break;
            case CAM_FREECAM:
                // todo
                break;
            default: break;
            }

            ImGui::RadioButton(("View##cameraV " + std::to_string(i)).c_str(), &eView, i);
            ImGui::SameLine();
            ImGui::RadioButton(("Control##cameraC " + std::to_string(i)).c_str(), &eControl, i);
            ImGui::SameLine();
            ImGui::RadioButton(("Both##cameraB " + std::to_string(i)).c_str(), &eBoth, i);

            if (cameraType == "Perspective")
            {
                ImGui::Checkbox(("Draw Frustum##" + std::to_string(i)).c_str(), &drawFrustum[i]);
                // Won't be visible on current view camera for obvious reasons
                if (drawFrustum[i] && currentCamera != cameraManager.GetCurrentCamera())
                {
                    static RendererModule& renderer = WindowsEngine::GetModule<RendererModule>();
                    DirectX::BoundingFrustum frustum = GetFrustumFromCamera(currentCamera);
                    renderer.DrawFrustum(frustum, Color{1, 1, 0});
                }
            }
        }

        if (eView != eViewCopy) { cameraManager.ChangeViewCamera(eView); }

        if (eControl != eControlCopy) { cameraManager.ChangeControlledCamera(eControl); }

        if (eBoth != eBothCopy)
        {
            cameraManager.ChangeViewCamera(eBoth);
            eView = eBoth;
            cameraManager.ChangeControlledCamera(eBoth);
            eControl = eBoth;
        }

        if (eView == eControl) { eBoth = eView; }
        else { eBoth = -1; }

        ImGui::Text((std::string{"Position : \nx: "} +
            std::to_string(transform.position.x) + std::string{" y:"} + std::to_string(transform.position.y) + std::string{" z:"} +
            std::to_string(transform.position.z)).c_str());

        const Vector3 rot = transform.rotation.ToEuler();

        ImGui::Text((std::string{"Rotation : \nx: "} +
            std::to_string(rot.x * 180 / DirectX::XM_PI) + std::string{" y:"} + std::to_string(rot.y * 180 / DirectX::XM_PI) + std::string{" z:"} +
            std::to_string(rot.z * 180 / DirectX::XM_PI)).c_str());

        ImGui::SeparatorText("Camera Settings: ");

        ImGui::Text("Speed: ");
        ImGui::SameLine();
        ImGui::DragFloat("##camera speed drag float", &cameraSpeed, 1, 0, 1000);

        projection->RenderImGui(this);

        return true;
    }

    return false;
#else
    return false;
#endif
}

}
