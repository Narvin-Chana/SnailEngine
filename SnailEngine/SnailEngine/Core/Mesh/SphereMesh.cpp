#include "stdafx.h"
#include "SphereMesh.h"

#include <algorithm>
#include <iterator>
#include <cmath>

#include "Core/WindowsEngine.h"
#include "Core/Physics/PhysicsModule.h"

namespace Snail
{
SphereMesh::SphereMesh(const unsigned int stacks, const unsigned int slices)
{
	std::vector<Vector3> vertexes;
	std::vector<Vector2> uvs;

	const uint16_t numStacks = static_cast<uint16_t>(stacks) + 1;
	const uint16_t numSlices = static_cast<uint16_t>(slices) + 1;

	for (unsigned int i = 0; i < numStacks; ++i)
	{
		const float phi = DirectX::XM_PI * static_cast<float>(i) / static_cast<float>(stacks);
		for (unsigned int j = 0; j < numSlices; ++j)
		{
			const float theta = 2.0f * DirectX::XM_PI * static_cast<float>(j) / static_cast<float>(slices);

			float x = std::sin(phi) * std::cos(theta);
			float y = std::cos(phi);
			float z = std::sin(phi) * std::sin(theta);

			vertexes.emplace_back(x, y, z);
			uvs.emplace_back(static_cast<float>(j) / static_cast<float>(slices), static_cast<float>(i) / static_cast<float>(stacks));
		}
	}

    // add quads per stack / slice
    for (uint16_t j = 0; j < stacks; j++)
    {
        const uint16_t j0 = j * static_cast<uint16_t>(numSlices);
        const uint16_t j1 = (j + 1) * static_cast<uint16_t>(numSlices);
        for (uint16_t i = 0; i < slices; i++)
        {
            uint16_t i0 = j0 + i;
            uint16_t i1 = j0 + (i + 1) % numSlices;
            uint16_t i2 = j1 + (i + 1) % numSlices;
            uint16_t i3 = j1 + i;

            indexes.insert(indexes.end(), {i0, i1, i2, i2, i3, i0,});
        }
    }

    std::transform(
        vertexes.begin(),
        vertexes.end(),
        uvs.begin(),
        std::back_inserter(vertices),
        [](const auto& vert, const auto& uv)
        {
            return MeshVertex{vert, vert, uv};
        }
    );

    SubMesh subMesh;
    subMesh.indexBufferCount = GetIndexCount();
    submeshes.push_back(std::move(subMesh));
}

}
