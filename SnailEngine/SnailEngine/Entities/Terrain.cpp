#include "stdafx.h"
#include "Terrain.h"

#include "TerrainChunk.h"
#include "Core/WindowsEngine.h"
#include "Core/Mesh/TerrainMesh.h"

using namespace physx;

namespace Snail
{
TerrainMesh* Terrain::GenerateChunkMesh(const int chunkX, const int chunkY, const TerrainMesh* tmesh) const
{
    static MeshManager& meshManager = WindowsEngine::GetModule<MeshManager>();

    std::unique_ptr<TerrainMesh> terrainMeshChunk = std::make_unique<TerrainMesh>();

    const uint32_t width = static_cast<uint32_t>(floor(static_cast<float>(tmesh->width) / chunkCount.x));
    const uint32_t height = static_cast<uint32_t>(floor(static_cast<float>(tmesh->height) / chunkCount.y));

    // Count for overlap on non-edges
    terrainMeshChunk->PopulateHeightField(width + (chunkX != chunkCount.x - 1), height + (chunkY != chunkCount.y - 1));

    auto mat = tmesh->submeshes[0].GetMaterial();
    const Image tex(mat.secondaryBlendTexture);
    
    // Add in the chunk's verts by extracting from terrainMesh
    for (uint32_t y = height * chunkY; y < height * (chunkY + 1) + (terrainMeshChunk->height - height); ++y)
    {
        for (uint32_t x = width * chunkX; x < width * (chunkX + 1) + (terrainMeshChunk->width - width); ++x)
        {
            MeshVertex vert = tmesh->vertices[x + y * tmesh->width];
            // set to between -1/chunkCount and 1/chunkCount
            vert.position.x -= 2.0f / chunkCount.x * chunkX;
            vert.position.z -= 2.0f / chunkCount.y * chunkY;
            // set to between 0 and 1/chunkCount
            vert.position.x = (vert.position.x + 1) / 2;
            vert.position.z = (vert.position.z + 1) / 2;

            const float ux = static_cast<float>(x) / tmesh->width;
            const float uy = static_cast<float>(y) / tmesh->height;

            const int imgX = static_cast<int>(ux * tex.GetWidth());
            const int imgY = static_cast<int>(uy * tex.GetHeight());

            const int val = tex.GetPixelInt(imgX, tex.GetHeight() - 1 - imgY).x;
            terrainMeshChunk->materialIds.push_back(val == 0 ? 0 : 1);
            terrainMeshChunk->vertices.push_back(vert);
        }
    }

    // Generate indices for a grid mesh (counter-clockwise triangles)
    for (uint32_t x = 0; x < terrainMeshChunk->width - 1; ++x)
    {
        for (uint32_t y = 0; y < terrainMeshChunk->height - 1; ++y)
        {
            uint32_t topLeft = x + y * terrainMeshChunk->width;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = x + (y + 1) * terrainMeshChunk->width;
            uint32_t bottomRight = bottomLeft + 1;

            // Define the two triangles for each quad face in counter-clockwise order
            terrainMeshChunk->indexes.push_back(topRight);
            terrainMeshChunk->indexes.push_back(topLeft);
            terrainMeshChunk->indexes.push_back(bottomLeft);

            terrainMeshChunk->indexes.push_back(topRight);
            terrainMeshChunk->indexes.push_back(bottomLeft);
            terrainMeshChunk->indexes.push_back(bottomRight);
        }
    }

    // Duplicate parent's material data
    SubMesh chunkSubMesh = tmesh->submeshes[0];
    chunkSubMesh.indexBufferCount = terrainMeshChunk->GetIndexCount();
    terrainMeshChunk->SetEnableBlending(tmesh->GetBlendingEnabled());
    terrainMeshChunk->submeshes.push_back(std::move(chunkSubMesh));

    return meshManager.SaveAsset<TerrainMesh>(entityName + "_chunk_" + std::to_string(chunkX * chunkCount.y + chunkY), std::move(terrainMeshChunk));
}

void Terrain::GenerateChunks(const TerrainMesh* sourceMesh)
{
    for (int chunkY = 0; chunkY < chunkCount.y; ++chunkY)
    {
        for (int chunkX = 0; chunkX < chunkCount.x; ++chunkX)
        {
            TerrainChunk::Params chunkParam;
            chunkParam.name = sourceMesh->name + "_chunk_" + std::to_string(chunkX * chunkCount.y + chunkY);
            chunkParam.castsShadows = castsShadows;
            auto* terrainMesh = GenerateChunkMesh(chunkX, chunkY, sourceMesh);
            chunkParam.mesh = terrainMesh;

            // Position for rendering relative to parents scale
            chunkParam.transform.position = Vector3{
                1.0f / chunkCount.x * chunkX - 0.5f,
                0,
                1.0f / chunkCount.y * chunkY - 0.5f
            };

            // chunkSize in world space
            chunkParam.chunkSize = Vector3{
                transform.scale.x / chunkCount.x,
                transform.scale.y,
                transform.scale.z / chunkCount.y
            };
            
            // Manual create without using CreateObject to pass parent
            std::unique_ptr<TerrainChunk> chunkEntity = std::make_unique<TerrainChunk>(chunkParam);
            chunkEntity->SetParent(this);
            chunkEntity->InitPhysics();
            chunks.push_back(std::move(chunkEntity));
        }
    }
}

Terrain::Params::Params()
{
    name = "Terrain";
    mesh = nullptr; // No default heightmap mesh as this not being defined is a catastrophic problem
    chunkSize = Vector2::One;
}

Terrain::Terrain(const Params& params)
    : Entity(params)
    , chunkCount(params.chunkSize)
{
    shouldFrustumCull = false;
    // Terrain without mesh makes no sense
    assert(mesh);
    // Shadow toggling isn't done cleanly, the renderer doesn't take into account child components when rendering.
    // Once (or if) an entity hierarchy is implemented, the castShadows of the chunks will be individually be taken into account.
    // For now I've changed each chunk to use its parent's castShadows bool and have made it toggleable in ImGui
    GenerateChunks(reinterpret_cast<TerrainMesh*>(mesh));

    // Delete main sourceMesh now that it has been chunked
    static MeshManager& meshManager = WindowsEngine::GetModule<MeshManager>();
    meshManager.DeleteAsset(mesh->name);
    mesh = nullptr;
}

void Terrain::Draw(DrawContext& ctx)
{
    for (const auto& chunk : chunks)
        chunk->Draw(ctx);
}

void Terrain::RenderImGui(const int idNumber)
{
#ifdef _IMGUI_
    ImGui::Checkbox(std::string("Casts Shadows: ##" + std::to_string(idNumber) + entityName).c_str(), &castsShadows);
    if (ImGui::TreeNodeEx(("Transform ##" + std::to_string(idNumber) + entityName).c_str()))
    {
        Vector3 position = transform.position;
        Vector3 rotation = transform.rotation.ToEuler();
        Vector3 scale = transform.scale;

        ImGui::Text("Position: ");
        if (ImGui::DragFloat3("##Position", reinterpret_cast<float*>(&position))) { SetPosition(position); }

        // This has some problems but can't be bothered to spend time fixing this.
        ImGui::Text("Rotation: ");
        if (ImGui::SliderAngle("##RotationX", &rotation.x, -89.9f, 89.9f, "x: %.0f deg") + ImGui::SliderAngle(
            "##RotationY",
            &rotation.y,
            -89.9f,
            89.9f,
            "y: %.0f deg") + ImGui::SliderAngle("##RotationZ", &rotation.z, -89.9f, 89.9f, "z: %.0f deg"))
        {
            SetRotation(rotation);
        }

        ImGui::Text("Scale: ");
        if (ImGui::DragFloat3("##Scale", reinterpret_cast<float*>(&scale))) { SetScale(scale); }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx(("Chunks ##" + std::to_string(idNumber) + entityName).c_str()))
    {
        for (int i = 0; i < chunks.size(); ++i)
        {
            if (ImGui::TreeNode(("Chunk " + std::to_string(i) + "##" + std::to_string(i)).c_str()))
            {
                chunks[i]->RenderImGui(idNumber);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
#else
    UNREFERENCED_PARAMETER(idNumber);
#endif
}

};
