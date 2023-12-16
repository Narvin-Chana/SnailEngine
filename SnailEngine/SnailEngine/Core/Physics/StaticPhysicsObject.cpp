#include "stdafx.h"
#include "StaticPhysicsObject.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

StaticPhysicsObject::StaticPhysicsObject(physx::PxShape* shape, const Transform& initialTransform)
{
    std::unique_lock lock{ PhysxMutex };

    StaticPhysicsObject::SetShape(shape);
    StaticPhysicsObject::SetTransform(initialTransform);
}

void StaticPhysicsObject::SetShape(physx::PxShape* shape)
{
    static PhysicsModule& physicsMod = WindowsEngine::GetModule<PhysicsModule>();
    body = physicsMod.physics->createRigidStatic(Transform{});
    body->attachShape(*shape);

    physicsMod.scene->addActor(*body);
}

bool StaticPhysicsObject::SetTransform(const Transform& transform)
{
    body->setGlobalPose(transform);
    return true;
}

void StaticPhysicsObject::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text("Physics Type: Static");
#endif
}
}
