#include "stdafx.h"
#include "TriggerBox.h"

#include "Core/RendererModule.h"
#include "Core/WindowsEngine.h"
#include <string>

#include "Core/Physics/StaticPhysicsObject.h"

namespace Snail
{
void TriggerBox::InitPhysics()
{
    static PhysicsModule& physModule = WindowsEngine::GetModule<PhysicsModule>();

    //create physics shape
    PhysXUniquePtr<PxShape> shape;

    Vector3 extents = Vector3::One / 2.0f;
    extents *= transform.scale;
    shape.reset(physModule.physics->createShape(PxBoxGeometry{extents.x, extents.y, extents.z}, *physModule.defaultMaterial));
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
    shape->userData = static_cast<void*>(triggerUD.get());
    physicsObject = std::make_unique<StaticPhysicsObject>(shape.get(), transform);
}

void TriggerBox::OnTriggerEnter()
{
    LOGF("Entered trigger \"{}\"", entityName);
}
void TriggerBox::OnTriggerExit()
{
    LOGF("Exited trigger \"{}\"", entityName);
}

void TriggerBox::Draw(DrawContext& ctx)
{
    if (triggerActive)
    {
        Entity::Draw(ctx);

#ifdef _DEBUG
        const DirectX::BoundingOrientedBox obb{ transform.position, transform.scale / 2, transform.rotation };
        ctx.renderer->DrawBoundingBox(obb, triggerColor);
#endif
    }
}

TriggerBox::Params::Params()
{
    castsShadows = false;
}

TriggerBox::TriggerBox(const Params& params)
    : Entity{params}
{}

std::string TriggerBox::GetJsonType()
{
    return "trigger_box";
}

TriggerBox::TriggerUserData::TriggerUserData(TriggerBox* trigger, Entity* entity)
    : UserData(entity)
    , triggerBox(trigger)
{ }

void TriggerBox::TriggerUserData::OnTrigger(TriggerBox* _triggerBox)
{
    if (!alreadyIn)
        _triggerBox->OnTriggerEnter();
    else
        _triggerBox->OnTriggerExit();

    alreadyIn = !alreadyIn;
}
}
