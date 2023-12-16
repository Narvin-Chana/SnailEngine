#include "stdafx.h"
#include "MeshManager.h"

#include <rapidobj.hpp>
#include <memory>

#include "Core/WindowsEngine.h"
#include "Core/Mesh/CubeMapMesh.h"
#include "Core/Mesh/CubeMesh.h"
#include "Core/Mesh/QuadMesh.h"
#include "Core/Mesh/SphereMesh.h"
#include "Core/Mesh/TerrainMesh.h"

namespace Snail
{
// Force generation of TerrainMesh version, otherwise a linker error will appear
template TerrainMesh* MeshManager::SaveAsset<TerrainMesh>(const std::string&, const std::string&, bool);
template Mesh<uint16_t>* MeshManager::SaveAsset<Mesh<uint16_t>>(const std::string&, const std::string&, bool);
template Mesh<uint32_t>* MeshManager::SaveAsset<Mesh<uint32_t>>(const std::string&, const std::string&, bool);

void MeshManager::Init()
{
    SaveAsset<CubeMesh>("Cube", std::make_unique<CubeMesh>(), true);
    SaveAsset<CubeMapMesh>("CubeMap", std::make_unique<CubeMapMesh>(), true);
    SaveAsset<QuadMesh>("Quad", std::make_unique<QuadMesh>(), true);
    SaveAsset<SphereMesh>("Sphere", std::make_unique<SphereMesh>(25, 25), true);
    SaveAsset<SphereMesh>("SphereSmall", std::make_unique<SphereMesh>(10, 10), true);
    SaveAsset<SphereMesh>("SphereBig", std::make_unique<SphereMesh>(50, 50), true);
}

void MeshManager::RenderImGui()
{
#ifdef _IMGUI_
    if (ImGui::CollapsingHeader("Meshes"))
    {
        for (auto& [meshName, mesh] : assetCache)
        {
            if (ImGui::TreeNode(meshName.c_str()))
            {
                mesh.asset->RenderImGui();

                ImGui::TreePop();
            }
        }
    }
#endif
}

}
