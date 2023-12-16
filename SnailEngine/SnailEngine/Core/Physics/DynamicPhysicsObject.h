#pragma once
#include <memory>

#include "PhysXAllocator.h"
#include "PhysicsObject.h"
#include "Core/Mesh/Mesh.h"

namespace physx {
	class PxRigidDynamic;
}

namespace Snail {

class DynamicPhysicsObject : public PhysicsObject {
public:
	DynamicPhysicsObject(physx::PxShape* shape, const Transform& initialTransform = {});

	void SetShape(physx::PxShape* shape) override;
	bool SetTransform(const Transform& transform) override;

    void RenderImGui() override;
};


}