#pragma once
#include <string>

#include "rapidobj.hpp"

#include "ModuleManager.h"
#include "Core/Mesh/Mesh.h"
#include "Util/RapidObjUtil.h"


namespace Snail
{
class MeshManager : public GenericAssetManager<BaseMesh>
{
    // Private load to cache for non-generic
    BaseMesh* SaveAsset(const std::string& filename, bool) override
    {
        LOGF(Logger::FATAL, "Saved base mesh \"{}\" to cache which is not allowed", filename);
        return nullptr;
    }
public:
    void Init() override;

    template<class T> requires std::is_base_of_v<BaseMesh, T>
    T* SaveAsset(const std::string& meshName, const std::string& filename, bool isPersistent = false);

    template<class T> requires std::is_base_of_v<BaseMesh, T>
    T* SaveAsset(const std::string& meshName, std::unique_ptr<T>&& mesh, bool isPersistent = false);
    

    void RenderImGui() override;
};


template <class T> requires std::is_base_of_v<BaseMesh, T>
T* MeshManager::SaveAsset(const std::string& meshName, const std::string& filename, bool isPersistent)
{
    std::string pathPrefix{};

    // Check if the last slash was found
    if (const size_t lastSlashPos = filename.find_last_of('/'); lastSlashPos != std::string::npos)
    {
        // Extract the substring from the beginning of the string up to the last '/'
        pathPrefix = filename.substr(0, lastSlashPos) + "/";
    }

    if (!std::filesystem::exists(filename))
    {
        LOG(Logger::FATAL, "Load error: terrain mesh file not found: ", filename);
        return nullptr;
    }

    rapidobj::Result result = rapidobj::ParseFile(filename);

    if (result.error)
    {
        LOG(Logger::FATAL, "Rapidobj: ", result.error.code.message());
        return nullptr;
    }

    if (!Triangulate(result))
    {
        LOG(Logger::ERROR, "Rapidobj: failed triangulation");
    }

    auto mesh = std::make_unique<T>();

    static constexpr auto hash = [](const rapidobj::Index& index) -> size_t
        {
            size_t h1 = std::hash<int>()(index.position_index);
            size_t h2 = std::hash<int>()(index.normal_index);
            size_t h3 = std::hash<double>()(index.texcoord_index);
            return h1 ^ h2 << 1 ^ h3;
        };

    auto comp = [](const rapidobj::Index& a, const rapidobj::Index& b) -> bool
        {
            return a.position_index == b.position_index && a.normal_index == b.normal_index && a.texcoord_index == b.texcoord_index;
        };

    std::unordered_map<rapidobj::Index, typename T::IndexType, decltype(hash), decltype(comp)> vertsIndex;

    // Process each shape
    for (const rapidobj::Shape& shape : result.shapes)
    {
        // Assuming that one shape = one material
        SubMesh subMesh{};

        subMesh.indexBufferStartIndex = static_cast<uint32_t>(mesh->indexes.size());
        // Create material from rapidobj material

        if (int materialIndex = shape.mesh.material_ids[0]; materialIndex != -1)
        {
            subMesh.SetMaterial(ConvertRapidObjMatToTexturedMaterial(result.materials[materialIndex], pathPrefix, isPersistent));
        }

        // Iterate through the faces of the shape
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
        {
            // Iterate through the vertices of the face
            // Use this loop for right handed winding:
            // for (int j = 0; j < 3; ++j)
            //
            // Use this one for left handed winding:
            for (int j = 2; j >= 0; --j)
            {
                const auto& indexes = shape.mesh.indices[i + j];
                const auto& [positionIndex, texcoordIndex, normalIndex] = indexes;

                Vector3 position = {
                result.attributes.positions[positionIndex * 3], result.attributes.positions[positionIndex * 3 + 1],
                result.attributes.positions[positionIndex * 3 + 2]
                };


                Vector3 normal{};
                if (normalIndex != -1)
                    normal = Vector3{
                        result.attributes.normals[normalIndex * 3], result.attributes.normals[normalIndex * 3 + 1],
                        result.attributes.normals[normalIndex * 3 + 2]
                };

                Vector2 uv{};
                if (texcoordIndex != -1)
                    uv = Vector2{
                        result.attributes.texcoords[texcoordIndex * 2],
                        result.attributes.texcoords[texcoordIndex * 2 + 1]
                };

                MeshVertex vertex{ position, normal, uv };

                // To convert right handed vertexes as left handed
                vertex.SwitchHandRule();

                auto index = static_cast<typename T::IndexType>(mesh->vertices.size());
                // Only duplicate vertex when the combo vertex normal and uv doesnt already exists.
                // This optimizes mesh verticies while still keeping uvs correctly
                if (vertsIndex.contains(indexes))
                {
                    index = vertsIndex[indexes];
                }
                else
                {
                    mesh->vertices.push_back(vertex);
                    vertsIndex[indexes] = index;
                }
                subMesh.indexBufferCount++;
                mesh->indexes.push_back(index);
            }
        }
        mesh->submeshes.push_back(subMesh);
    }

    return SaveAsset(meshName, std::move(mesh), isPersistent);
}



template <class T> requires std::is_base_of_v<BaseMesh, T>
T* MeshManager::SaveAsset(const std::string& meshName, std::unique_ptr<T>&& mesh, bool isPersistent)
{
    assert(mesh != nullptr);
    mesh->name = meshName;
    mesh->Init();

    std::lock_guard _{ assetManagerMutex };
    assetCache[meshName] = { isPersistent, std::move(mesh) };
    LOGF("Saved mesh \"{}\" to cache", meshName);
    return dynamic_cast<T*>(assetCache[meshName].asset.get());
}

}
