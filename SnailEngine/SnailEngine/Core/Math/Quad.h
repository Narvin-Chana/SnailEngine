#pragma once

struct Quad
{
    Vector2 topLeft, topRight, botLeft, botRight;

    bool Insersect(const Vector2& pt) const;
};

inline bool Quad::Insersect(const Vector2& pt) const
{
    const auto posIn = pt - topLeft;
    const auto posOut = botRight - topLeft;
    return posIn.x > 0 && posIn.y > 0 && posIn.x <= posOut.x && posIn.y <= posOut.y;
}
