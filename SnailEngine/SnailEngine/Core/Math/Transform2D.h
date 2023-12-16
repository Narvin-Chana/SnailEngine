#pragma once
#include "Util/FormatUtil.h"

namespace Snail
{
struct Transform2D
{
    Vector2 position{0,0};
    Vector2 scale{1,1};
    float rotation{};

    Matrix GetTransformMatrix() const;
};
}

template <>
struct std::formatter<Snail::Transform2D> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(const Snail::Transform2D& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "pos = {}, rot = {}, scale = {}", obj.position, obj.rotation, obj.scale);
    }
};