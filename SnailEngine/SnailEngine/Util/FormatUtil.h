#pragma once
#include <format>
#include <Core/Math/SimpleMath.h>

template <>
struct std::formatter<Vector2>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const Vector2& vec, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{} {}", vec.x, vec.y);
    }
};

template <>
struct std::formatter<Vector3>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const Vector3& vec, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{} {} {}", vec.x, vec.y, vec.z);
    }
};

template <>
struct std::formatter<Vector4>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const Vector4& vec, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{} {} {} {}", vec.x, vec.y, vec.z, vec.w);
    }
};

template <>
struct std::formatter<Quaternion>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const Quaternion& quat, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{} {} {} {}", quat.x, quat.y, quat.z, quat.w);
    }
};
