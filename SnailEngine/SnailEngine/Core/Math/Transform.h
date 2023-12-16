#pragma once

#include <PxPhysicsAPI.h>
#include <json.hpp>
#include <format>
#include <tuple>

#include "Util/FormatUtil.h"

namespace Snail
{
struct Transform
{
    // SnailEngine uses left-handed chirality
    // We also use Y for up
    Vector3 position = Vector3::Zero; // X, Y, Z
    Quaternion rotation = Quaternion::Identity; // Quaternion
    Vector3 scale = Vector3::One; // X, Y, Z

    Matrix GetTransformationMatrix() const noexcept;
    Transform Combine(const Transform& other) const;

    Vector3 GetForwardVector() const noexcept;
    Vector3 GetUpVector() const noexcept;
    Vector3 GetRightVector() const noexcept;

    bool RenderImGui();
    void LookAt(const Vector3& target, const Vector3& up);

    // Better when the three are needed { up, right, forward }
    std::tuple<Vector3, Vector3, Vector3> GetAxis() const noexcept;

    operator physx::PxTransform() const noexcept;
    void UpdateFromPhysics(const physx::PxTransform& transform);
};
}

template <>
struct std::formatter<Snail::Transform> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(const Snail::Transform& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "pos = {}, rot = {}, scale = {}", obj.position, obj.rotation, obj.scale);
    }
};
