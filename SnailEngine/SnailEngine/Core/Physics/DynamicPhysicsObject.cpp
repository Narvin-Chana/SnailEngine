#include "stdafx.h"
#include "DynamicPhysicsObject.h"

#include <PxRigidDynamic.h>

#include "Core/WindowsEngine.h"

namespace Snail {

DynamicPhysicsObject::DynamicPhysicsObject(physx::PxShape* shape, const Transform& initialTransform)
{
    std::unique_lock lock{ PhysxMutex };

    DynamicPhysicsObject::SetShape(shape);
    body->setGlobalPose(initialTransform);
}

void DynamicPhysicsObject::SetShape(physx::PxShape* shape)
{
    static PhysicsModule& physicsMod = WindowsEngine::GetModule<PhysicsModule>();
	body = physicsMod.physics->createRigidDynamic(Transform{});
	body->attachShape(*shape);

	physicsMod.scene->addActor(*body);
}

bool DynamicPhysicsObject::SetTransform(const Transform& transform)
{
    body->setGlobalPose(transform);
    return true;
}

void DynamicPhysicsObject::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text("Physics Type: Dynamic");
#endif
}
}

