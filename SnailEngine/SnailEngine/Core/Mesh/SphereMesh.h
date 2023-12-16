#pragma once
#include "Mesh.h"

namespace Snail
{

class SphereMesh : public Mesh<uint16_t>
{
    using Mesh::indexes;
    using Mesh::vertices;
public:
	SphereMesh(unsigned int stacks, unsigned int slices);
};

}
