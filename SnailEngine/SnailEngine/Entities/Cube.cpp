#include "stdafx.h"

#include "Cube.h"

#include "Core/WindowsEngine.h"
#include "Core/Mesh/CubeMesh.h"

namespace Snail
{
Cube::Params::Params()
{
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    name = "Cube";
    mesh = mm.GetAsset<CubeMesh>("Cube");
}

Cube::Cube(const Params& params)
	: Entity(params)
{
}

std::string Cube::GetJsonType()
{
    return "cube";
}
}
