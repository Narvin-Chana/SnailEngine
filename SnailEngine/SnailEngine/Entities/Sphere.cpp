#include "stdafx.h"

#include "Sphere.h"

#include <DirectXMath.h>
#include <PxPhysicsAPI.h>

#include "Core/Mesh/SphereMesh.h"
#include "Core/WindowsEngine.h"

using namespace physx;

namespace Snail
{
Sphere::Params::Params()
{
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    name = "Sphere";
    mesh = mm.GetAsset<SphereMesh>("Sphere");
}

Sphere::Sphere(const Params& params)
    : Entity(params)
{}

std::string Sphere::GetJsonType()
{
    return "sphere";
}
};
