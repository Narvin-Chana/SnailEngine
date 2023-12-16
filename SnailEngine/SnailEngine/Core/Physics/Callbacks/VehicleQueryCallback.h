#pragma once
#include <PxBaseMaterial.h>
#include <PxQueryFiltering.h>
#include <PxShape.h>
#include <PxMaterial.h>

struct VehicleQueryCallback : physx::PxQueryFilterCallback
{
    physx::PxQueryHitType::Enum preFilter(
        const physx::PxFilterData&,
        const physx::PxShape* shape,
        const physx::PxRigidActor*,
        physx::PxHitFlags&) override
    {
        // Check if the shape is a trigger
        // If it's a trigger, skip the query (no hit)
        if (shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
            return physx::PxQueryHitType::eNONE;

        return physx::PxQueryHitType::eBLOCK;
    }

    physx::PxQueryHitType::Enum postFilter(
        const physx::PxFilterData&,
        const physx::PxQueryHit& hit,
        const physx::PxShape* shape,
        const physx::PxRigidActor*) override
    {
        // Check if the shape is a trigger
        // If it's a trigger, skip further processing (no hit)
        if (shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
            return physx::PxQueryHitType::eNONE;

        if (hit.faceIndex != 0xFFFFffff)
        {
            physx::PxMaterial* material = shape->getMaterialFromInternalFaceIndex(hit.faceIndex)->is<physx::PxMaterial>();
            if (material->userData)
                ((void(*)())material->userData)();
        }

        return physx::PxQueryHitType::eBLOCK;
    }
};
