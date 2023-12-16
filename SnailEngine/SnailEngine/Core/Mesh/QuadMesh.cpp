#include "stdafx.h"
#include "QuadMesh.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
QuadMesh::QuadMesh()
{
    // Define quad vertices
    MeshVertex v0{}, v1{}, v2{}, v3{};

    v0.position = Vector3{-0.5f, 0.5f, 0.0f}; // Top-left
    v0.normal = Vector3(0, 0, 1);
    v0.uv = Vector2{1.0f, 0.0f};

    v1.position = Vector3{0.5f, 0.5f, 0.0f}; // Top-right
    v1.normal = Vector3(0, 0, 1);
    v1.uv = Vector2{0.0f, 0.0f};

    v2.position = Vector3{0.5f, -0.5f, 0.0f}; // Bottom-right
    v2.normal = Vector3(0, 0, 1);
    v2.uv = Vector2{0.0f, 1.0f};

    v3.position = Vector3{-0.5f, -0.5f, 0.0f}; // Bottom-left
    v3.normal = Vector3(0, 0, 1);
    v3.uv = Vector2{1.0f, 1.0f};

    // Add vertices to the vector
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);

    // Define quad indices (two triangles counter-clockwise)
    indexes.push_back(0); // Top-left
    indexes.push_back(2); // Bottom-right
    indexes.push_back(1); // Top-right

    indexes.push_back(0); // Top-left
    indexes.push_back(3); // Bottom-left
    indexes.push_back(2); // Bottom-right

    SubMesh subMesh;
    subMesh.indexBufferCount = GetIndexCount();
    submeshes.push_back(std::move(subMesh));
}

}
