#include "stdafx.h"
#include "PhysicsObject.h"

#include "Core/WindowsEngine.h"

namespace Snail {

PhysicsObject::~PhysicsObject()
{
    static PhysicsModule& physicsMod = WindowsEngine::GetModule<PhysicsModule>();
    if (body)
    {
        physicsMod.scene->removeActor(*body);
    }
}

void PhysicsObject::SetUserData(Entity::UserData* userData)
{
    body->userData = static_cast<void*>(userData);
}

void PhysicsObject::UpdateTransform(Transform& transform)
{
    transform.UpdateFromPhysics(body->getGlobalPose());
}

}
