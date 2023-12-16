#include "stdafx.h"
#include "CubeMesh.h"

#include "Core/WindowsEngine.h"
#include "Core/Physics/PhysicsModule.h"

namespace Snail {

CubeMesh::CubeMesh()
{
	indexes = {
		0,1,2, // front
		0,2,3, // front

		5,6,7, // back
		5,7,4, // back

		8,9,10, // down
		8,10,11, // down

		13,14,15, // up
		13,15,12, // up

		19,16,17, // left
		19,17,18, // left

		20,21,22, // right
		20,22,23 // right
	};

    constexpr float dx = 1;
    constexpr float dy = 1;
    constexpr float dz = 1;

	// Les points
    constexpr Vector3 point[8] =
	{
		Vector3(-dx / 2, dy / 2, -dz / 2),
		Vector3(dx / 2, dy / 2, -dz / 2),
		Vector3(dx / 2, -dy / 2, -dz / 2),
		Vector3(-dx / 2, -dy / 2, -dz / 2),
		Vector3(-dx / 2, dy / 2, dz / 2),
		Vector3(-dx / 2, -dy / 2, dz / 2),
		Vector3(dx / 2, -dy / 2, dz / 2),
		Vector3(dx / 2, dy / 2, dz / 2)
	};

	// Calculer les normales
    constexpr Vector3 n0(0.0f, 0.0f, -1.0f); // front
    constexpr Vector3 n1(0.0f, 0.0f, 1.0f); // back
    constexpr Vector3 n2(0.0f, -1.0f, 0.0f); // down
    constexpr Vector3 n3(0.0f, 1.0f, 0.0f); // up
    constexpr Vector3 n4(-1.0f, 0.0f, 0.0f); // left
    constexpr Vector3 n5(1.0f, 0.0f, 0.0f); // right

	vertices.reserve(24);

	// Le devant du bloc
	vertices.emplace_back(point[0], n0, Vector2{ 0, 0 });
	vertices.emplace_back(point[1], n0, Vector2{ 1, 0 });
	vertices.emplace_back(point[2], n0, Vector2{ 1, 1 });
	vertices.emplace_back(point[3], n0, Vector2{ 0, 1 });
	// L�arri�re du bloc
	vertices.emplace_back(point[4], n1, Vector2{ 0, 1 });
	vertices.emplace_back(point[5], n1, Vector2{ 0, 0 });
	vertices.emplace_back(point[6], n1, Vector2{ 1, 0 });
	vertices.emplace_back(point[7], n1, Vector2{ 1, 1 });
	// Le dessous du bloc
	vertices.emplace_back(point[3], n2, Vector2{ 0, 0 });
	vertices.emplace_back(point[2], n2, Vector2{ 1, 0 });
	vertices.emplace_back(point[6], n2, Vector2{ 1, 1 });
	vertices.emplace_back(point[5], n2, Vector2{ 0, 1 });
	// Le dessus du bloc
	vertices.emplace_back(point[0], n3, Vector2{ 0, 1 });
	vertices.emplace_back(point[4], n3, Vector2{ 0, 0 });
	vertices.emplace_back(point[7], n3, Vector2{ 1, 0 });
	vertices.emplace_back(point[1], n3, Vector2{ 1, 1 });
	// La face gauche
	vertices.emplace_back(point[0], n4, Vector2{ 0, 0 });
	vertices.emplace_back(point[3], n4, Vector2{ 1, 0 });
	vertices.emplace_back(point[5], n4, Vector2{ 1, 1 });
	vertices.emplace_back(point[4], n4, Vector2{ 0, 1 });
	// La face droite
	vertices.emplace_back(point[1], n5, Vector2{ 0, 0 });
	vertices.emplace_back(point[7], n5, Vector2{ 1, 0 });
	vertices.emplace_back(point[6], n5, Vector2{ 1, 1 });
	vertices.emplace_back(point[2], n5, Vector2{ 0, 1 });

    SubMesh subMesh;
    subMesh.indexBufferCount = GetIndexCount();
    submeshes.push_back(std::move(subMesh));
}

}
