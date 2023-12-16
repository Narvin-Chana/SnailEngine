#pragma once
#include "Core/Math/Transform.h"
#include "Entities/Entity.h"

namespace physx
{
class PxShape;
}

namespace Snail
{

class PhysicsObject
{
protected:
    physx::PxRigidActor* body = nullptr;

public:
    // This is not thread safe
    virtual void SetShape(physx::PxShape* shape) = 0;
    virtual bool SetTransform(const Transform& transform) = 0;
    void SetUserData(Entity::UserData* userData);

    virtual void UpdateTransform(Transform& transform);
    virtual ~PhysicsObject();
    virtual void RenderImGui() = 0;
    physx::PxRigidActor* getBody() const { return body; };
};
}
