#include "stdafx.h"

#include "MenuMesh.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
MenuMesh::Params::Params()
{
    turnSpeed = 0.0f;
    rotationAxis = Vector3::Up;
    name = "MenuMesh";
}

MenuMesh::MenuMesh(const Params& params)
    : Entity(params)
    , turnSpeed{params.turnSpeed}
    , rotationAxis{params.rotationAxis}
{}

void MenuMesh::Update(const float dt) noexcept
{
    Entity::Update(dt);

    if (turnSpeed != 0.0f && rotationAxis != Vector3::Zero)
    {
        transform.rotation = DirectX::XMQuaternionMultiply(transform.rotation, DirectX::XMQuaternionRotationAxis(rotationAxis, turnSpeed * dt));

        SetTransform(transform);
    }
}

std::string MenuMesh::GetJsonType()
{
    return "menuMesh";
}
}
