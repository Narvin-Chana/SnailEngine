#include "stdafx.h"
#include "MeshVertex.h"

namespace Snail
{
MeshVertex::MeshVertex(const Vector3& position, const Vector3& normal, const Vector2& uv)
    : position{position}
    , normal{normal}
    , uv{uv}
{}

void MeshVertex::SwitchHandRule()
{
    position.z *= -1;
    bitangent.z *= -1;
    tangent.z *= -1;
    normal.z *= -1;
    uv.y = 1 - uv.y;
}

}
