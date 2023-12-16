#pragma once
#include "PhysicsObject.h"
#include "Core/Mesh/Mesh.h"

namespace Snail {

class StaticPhysicsObject : public PhysicsObject {
public:
	StaticPhysicsObject(physx::PxShape* shape, const Transform& initialTransform = {});

    template<class T>
    StaticPhysicsObject(const Mesh<T>& mesh, const Transform& initialTransform = {});

	void SetShape(physx::PxShape* shape) override;
	bool SetTransform(const Transform& transform) override;

    void RenderImGui() override;
};

template<class T>
StaticPhysicsObject::StaticPhysicsObject(const Mesh<T>& mesh, const Transform& initialTransform)
{
    if (physx::PxShape* shape = mesh.GetPhysicsShape(initialTransform.scale))
    {
        StaticPhysicsObject::SetShape(shape);
        StaticPhysicsObject::SetTransform(initialTransform);
    }
}

}