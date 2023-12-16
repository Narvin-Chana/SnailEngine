#pragma once

#include <PxPhysicsAPI.h>

#include "Entities/Triggers/TriggerBox.h"

using namespace physx;

namespace Snail
{
class ContactCallback : public PxSimulationEventCallback, public PxContactModifyCallback
{
public:
    void onTrigger(PxTriggerPair* pairs, PxU32 count) override
    {
        for (PxU32 i = 0; i < count; i++)
        {
            // ignore pairs when shapes have been deleted
            if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
                continue;

            if (pairs[i].triggerShape->userData)
            {
                auto* triggerData = static_cast<TriggerBox::TriggerUserData*>(pairs[i].triggerShape->userData);
                triggerData->OnTrigger(triggerData->triggerBox);

                if (pairs[i].otherShape->userData)
                {
                    auto* userData = static_cast<Entity::UserData*>(pairs[i].otherShape->userData);
                    userData->OnTrigger(triggerData->triggerBox);
                }
            }
        }
    }

    void onContact(const PxContactPairHeader&, const PxContactPair*, PxU32) override
    { }

    void onContactModify(PxContactModifyPair* const, PxU32) override
    { }

    void onConstraintBreak(PxConstraintInfo*, PxU32) override
    { }

    void onWake(PxActor**, PxU32) override
    { }

    void onSleep(PxActor**, PxU32) override
    { }

    void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) override
    { }
};

PxFilterFlags FilterShader(
    PxFilterObjectAttributes attributes0,
    PxFilterData,
    PxFilterObjectAttributes attributes1,
    PxFilterData,
    PxPairFlags& pairFlags,
    const void*,
    PxU32)
{
    // Check if either object is a trigger
    const bool isTrigger0 = PxFilterObjectIsTrigger(attributes0);
    const bool isTrigger1 = PxFilterObjectIsTrigger(attributes1);

    // If both are triggers, no interaction is needed
    if (isTrigger0 && isTrigger1)
    {
        return PxFilterFlag::eSUPPRESS;
    }

    // Let triggers through
    if (isTrigger0 || isTrigger1)
    {
        pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
        return PxFilterFlag::eDEFAULT;
    }

    pairFlags = PxPairFlag::eCONTACT_DEFAULT;

    return PxFilterFlag::eDEFAULT;
}
}
