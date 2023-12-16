#include "stdafx.h"
#include "Transform.h"

#include "Util/JsonUtil.h"

namespace Snail
{

Transform Transform::Combine(const Transform& other) const
{
    Matrix combinationMatrix = GetTransformationMatrix() * other.GetTransformationMatrix();

    Transform retTransform;
    combinationMatrix.Decompose(retTransform.scale, retTransform.rotation, retTransform.position);

    return retTransform;
}

Matrix Transform::GetTransformationMatrix() const noexcept
{
    return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
}

Vector3 Transform::GetForwardVector() const noexcept
{
    return Vector3::Transform(Vector3::UnitZ, rotation);
}

Vector3 Transform::GetUpVector() const noexcept
{
    return Vector3::Transform(Vector3::UnitY, rotation);
}

Vector3 Transform::GetRightVector() const noexcept
{
    return Vector3::Transform(Vector3::UnitX, rotation);
}

bool Transform::RenderImGui()
{
#ifdef _IMGUI_
    Vector3 oldPosition = position;
    Vector3 oldRotation{};
    Vector3 oldScale = scale;
    bool changedAnyValue = false;
    ImGui::Text("Position: ");
    if (ImGui::DragFloat3("##Position", &oldPosition.x, 0.01f))
    {
        changedAnyValue = true;
        position = oldPosition;
    }

    // This has some problems but can't be bothered to spend time fixing this.
    ImGui::Text("Rotation: ");
    if (ImGui::DragFloat3("##Rotation", &oldRotation.x, 0.01f, 0, 0))
    {
        changedAnyValue = true;
        rotation *= Quaternion::CreateFromYawPitchRoll(oldRotation);
    }

    if (ImGui::Button("Reset Rotation##resetRot"))
    {
        changedAnyValue = true;
        rotation = Quaternion::Identity;
    }

    ImGui::Text("Scale: ");
    if (ImGui::DragFloat3("##Scale", &oldScale.x, 0.01f))
    {
        changedAnyValue = true;
        scale = oldScale;
    }

    return changedAnyValue;
#else
    return false;
#endif
}

void Transform::LookAt(const Vector3& target, const Vector3& up)
{
    Vector3 forward = target - position;
    forward.Normalize();
    Vector3 right = up.Cross(forward);
    right.Normalize();
    Vector3 newUp = forward.Cross(right);
    newUp.Normalize();

    rotation = Quaternion::LookRotation(forward, newUp);
}

std::tuple<Vector3, Vector3, Vector3> Transform::GetAxis() const noexcept
{
    return {GetUpVector(), GetRightVector(), GetForwardVector()};
}

Transform::operator physx::PxTransform() const noexcept
{
    physx::PxTransform transform;

    transform.q = physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w);
    transform.p = physx::PxVec3(position.x, position.y, position.z);

    return transform;
}

void Transform::UpdateFromPhysics(const physx::PxTransform& transform)
{
    position = Vector3{transform.p.x, transform.p.y, transform.p.z};
    rotation = Quaternion{transform.q.x, transform.q.y, transform.q.z, transform.q.w};
}

}
