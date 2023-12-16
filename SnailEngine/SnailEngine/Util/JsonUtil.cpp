#include "stdafx.h"

#include "JsonUtil.h"

#include "Core/Mesh/CubeMapMesh.h"
#include "Core/Mesh/CubeMesh.h"
#include "Core/Mesh/DecalMesh.h"
#include "Core/Mesh/QuadMesh.h"
#include "Core/Mesh/SphereMesh.h"
#include "Core/Mesh/TerrainMesh.h"
#include "Core/Physics/DynamicPhysicsObject.h"
#include "Core/Physics/StaticPhysicsObject.h"
#include "Entities/Billboard.h"
#include "Entities/CubeSkybox.h"
#include "Entities/Cube.h"
#include "Entities/Decal.h"
#include "Entities/Sphere.h"
#include "Entities/Terrain.h"
#include "Entities/Entity.h"
#include "Entities/InstancedEntity.h"
#include "Entities/Firefly.h"
#include "Entities/Vehicle.h"
#include "Entities/Triggers/CheckpointTrigger.h"
#include "Entities/Triggers/TriggerBox.h"
#include "Entities/MenuMesh.h"
#include "Rendering/TexturedMaterial.h"

namespace Snail
{
template <>
void from_json(const nlohmann::json& json, Entity::Params& p)
{
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();
    static PhysicsModule& pm = WindowsEngine::GetModule<PhysicsModule>();

    get_to_if_exists(json, "name", p.name);
    get_to_if_exists(json, "transform", p.transform);

    if (std::string meshName; get_to_if_exists(json, "mesh", meshName))
    {
        p.mesh = mm.GetAsset<BaseMesh>(meshName);
    }

    if (nlohmann::basic_json jPhysics; get_to_if_exists(json, "physics", jPhysics))
    {
        std::string type = "static";
        get_to_if_exists(jPhysics, "type", type);

        Transform physicsTransform{};
        get_to_if_exists(jPhysics, "shape_transform", physicsTransform);
        physicsTransform.scale *= p.transform.scale;

        const std::string shapeType = jPhysics.at("shape").get<std::string>();

        PhysXUniquePtr<physx::PxShape> shape{};
        if (shapeType == "box")
        {
            Vector3 extents = Vector3::One / 2.0f;
            get_to_if_exists(jPhysics, "extents", extents);
            extents *= physicsTransform.scale;
            shape.reset(pm.physics->createShape(physx::PxBoxGeometry{extents.x, extents.y, extents.z}, *pm.defaultMaterial));
        }
        else if (shapeType == "sphere")
        {
            float radius = 1;
            get_to_if_exists(jPhysics, "radius", radius);
            radius *= physicsTransform.scale.x;

            shape.reset(pm.physics->createShape(physx::PxSphereGeometry{radius}, *pm.defaultMaterial));
        }
        else if (shapeType == "capsule")
        {
            float radius = 1;
            get_to_if_exists(jPhysics, "radius", radius);
            radius *= physicsTransform.scale.x;

            float halfHeight = 1;
            get_to_if_exists(jPhysics, "half_height", halfHeight);
            halfHeight *= physicsTransform.scale.y;
            shape.reset(pm.physics->createShape(physx::PxCapsuleGeometry{radius, halfHeight}, *pm.defaultMaterial));
        }
        else if (shapeType == "plane")
        {
            shape.reset(pm.physics->createShape(physx::PxPlaneGeometry(), *pm.defaultMaterial));
        }
        else if (shapeType == "mesh")
        {
            // Default to mesh of the entity
            BaseMesh* physicsMesh = p.mesh;

            if (std::string meshName; get_to_if_exists(jPhysics, "mesh_name", meshName))
            {
                physicsMesh = mm.GetAsset<BaseMesh>(meshName);
            }

            if (physicsMesh)
            {
                std::string meshType = "convex";
                get_to_if_exists(jPhysics, "mesh_type", meshType);

                if (meshType == "convex")
                {
                    shape.reset(pm.GenerateMeshConvexShape(static_cast<Mesh<uint32_t>*>(physicsMesh), physicsTransform.scale));
                }
                else if (meshType == "triangle")
                {
                    if (type != "dynamic")
                    {
                        if (const auto* physMesh = dynamic_cast<Mesh<uint32_t>*>(physicsMesh))
                            shape.reset(pm.GenerateMeshTriangleShape(physMesh, physicsTransform.scale));
                        else if (const auto* physMesh16 = dynamic_cast<Mesh<>*>(physicsMesh))
                            shape.reset(pm.GenerateMeshTriangleShape(physMesh16, physicsTransform.scale));
                    }
                    else
                    {
                        LOGF(Logger::FATAL, "Dynamic physics triangle meshes are not supported");
                    }
                }
            }
        }

        if (shape)
        {
            shape->setLocalPose(physicsTransform);
            if (type == "dynamic")
                p.physicsObject = new DynamicPhysicsObject{shape.get(), p.transform};
            else if (type == "static")
                p.physicsObject = new StaticPhysicsObject{shape.get(), p.transform};
        }
        else
        {
            LOGF(Logger::WARN, "Physics shape wasn't created when physics description was supplied for entity \"{}\"", p.name);
        }
    }
}

/*
TODO: Find a way to do whats here instead of whats above. This would prevent code duplication

template<class T>
void from_json(const nlohmann::json& j, typename Entity<T>::Params& p)
{
    from_json(j, static_cast<Entity::Params&>(p));
    get_to_if_exists(j, "physics", p.physicsType);
}

template void from_json<Entity::Params>(const nlohmann::json& j, Entity::Params& p);
template void from_json<Entity::Params>(const nlohmann::json& j, Entity::Params& p);*/

template <>
void from_json(const nlohmann::json& json, TexturedMaterial& material)
{
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();

    // json crashes when getting directly to wstring
    // Solution is to use string instead
    // But this means that some parsing problems could happen
    // So avoid using weird characters in texture filepath...

    if (get_to_if_exists(json, "diffuse_filepath", material.diffuseTexture))
    {
        tm.SaveAsset<Texture2D>(material.diffuseTexture);
    }
    if (get_to_if_exists(json, "primary_blend_diffuse_filepath", material.primaryBlendDiffuseTexture))
    {
        material.isBlending = true;
        tm.SaveAsset<Texture2D>(material.primaryBlendDiffuseTexture);
    }
    if (get_to_if_exists(json, "primary_blend_filepath", material.primaryBlendTexture))
    {
        material.isBlending = true;
        tm.SaveAsset<Texture2D>(material.primaryBlendTexture);
    }
    if (get_to_if_exists(json, "secondary_blend_diffuse_filepath", material.secondaryBlendDiffuseTexture))
    {
        material.isBlending = true;
        tm.SaveAsset<Texture2D>(material.secondaryBlendDiffuseTexture);
    }
    if (get_to_if_exists(json, "secondary_blend_filepath", material.secondaryBlendTexture))
    {
        material.isBlending = true;
        tm.SaveAsset<Texture2D>(material.secondaryBlendTexture);
    }
    if (get_to_if_exists(json, "ambient_filepath", material.ambientTexture))
    {
        tm.SaveAsset<Texture2D>(material.ambientTexture);
    }
    if (get_to_if_exists(json, "specular_filepath", material.specularTexture))
    {
        tm.SaveAsset<Texture2D>(material.specularTexture);
    }
    if (get_to_if_exists(json, "normalmap_filepath", material.normalMapTexture))
    {
        tm.SaveAsset<Texture2D>(material.normalMapTexture);
    }

    get_to_if_exists(json, "parameters", material.material);
}

template <>
void from_json(const nlohmann::json& json, Material& materialparam)
{
    get_to_if_exists(json, "diffuse", materialparam.diffuse);
    get_to_if_exists(json, "ambient", materialparam.ambient);
    get_to_if_exists(json, "emission", materialparam.emission);
    get_to_if_exists(json, "specular", materialparam.specular);
    get_to_if_exists(json, "shininess", materialparam.shininess);
    get_to_if_exists(json, "uv_scale", materialparam.uvScale);
    get_to_if_exists(json, "primary_blend_uv_scale", materialparam.primaryBlendUvScale);
    get_to_if_exists(json, "secondary_blend_uv_scale", materialparam.primaryBlendUvScale);
}

template <>
void from_json(const nlohmann::json& json, Terrain::Params& p)
{
    if (json.contains("physics"))
        LOGF(Logger::WARN, "Physics on terrain will be overriden by terrain physics");

    from_json(json, static_cast<Entity::Params&>(p));
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();

    const std::string terrainMeshFile = json.at("mesh_filepath").get<std::string>();

    uint32_t width = 0, height = 0;
    get_to_if_exists(json, "width", width);
    get_to_if_exists(json, "height", height);

    TerrainMesh* terrainMesh = mm.SaveAsset<TerrainMesh>(terrainMeshFile, terrainMeshFile);

    get_to_if_exists(json, "chunk_count", p.chunkSize);
    terrainMesh->PopulateHeightField(width, height);

    if (std::string terrainDiffuseFile; get_to_if_exists(json, "diffuse_filepath", terrainDiffuseFile))
    {
        tm.SaveAsset<Texture2D>(terrainDiffuseFile);
        terrainMesh->SetAllMaterialMember(terrainDiffuseFile, &TexturedMaterial::diffuseTexture);
    }

    if (std::string blendDiffuseFilepath; get_to_if_exists(json, "primary_blend_diffuse_filepath", blendDiffuseFilepath))
    {
        tm.SaveAsset<Texture2D>(blendDiffuseFilepath);
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetAllMaterialMember(blendDiffuseFilepath, &TexturedMaterial::primaryBlendDiffuseTexture);
    }

    if (std::string blendFile; get_to_if_exists(json, "primary_blend_filepath", blendFile))
    {
        tm.SaveAsset<Texture2D>(blendFile);
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetAllMaterialMember(blendFile, &TexturedMaterial::primaryBlendTexture);
    }

    if (std::string blendDiffuseFilepath; get_to_if_exists(json, "secondary_blend_diffuse_filepath", blendDiffuseFilepath))
    {
        tm.SaveAsset<Texture2D>(blendDiffuseFilepath);
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetAllMaterialMember(blendDiffuseFilepath, &TexturedMaterial::secondaryBlendDiffuseTexture);
    }

    if (std::string blendFile; get_to_if_exists(json, "secondary_blend_filepath", blendFile))
    {
        tm.SaveAsset<Texture2D>(blendFile);
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetAllMaterialMember(blendFile, &TexturedMaterial::secondaryBlendTexture);
    }

    if (std::string normalMapFilepath; get_to_if_exists(json, "normalmap_filepath", normalMapFilepath))
    {
        tm.SaveAsset<Texture2D>(normalMapFilepath);
        terrainMesh->SetAllMaterialMember(normalMapFilepath, &TexturedMaterial::normalMapTexture);
    }

    if (Vector2 uvScale; get_to_if_exists(json, "uv_scale", uvScale))
    {
        terrainMesh->SetUVScale(uvScale);
    }

    if (Vector2 blendUVScale; get_to_if_exists(json, "secondary_blend_uv_scale", blendUVScale))
    {
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetPrimaryBlendUVScale(blendUVScale);
    }

    if (Vector2 blendUVScale; get_to_if_exists(json, "secondary_blend_uv_scale", blendUVScale))
    {
        terrainMesh->SetEnableBlending(true);
        terrainMesh->SetSecondaryBlendUVScale(blendUVScale);
    }

    terrainMesh->SortVertices();
    p.mesh = terrainMesh;
    p.physicsObject = nullptr;
}

template <>
void from_json(const nlohmann::json& json, CubeSkybox::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();

    if (std::string tex; get_to_if_exists(json, "texture", tex))
    {
        CubeMapMesh* mesh = mm.GetAsset<CubeMapMesh>("CubeMap");
        mesh->SetCullingType(BaseMesh::CullingType::FRONT);

        SubMesh subMesh;
        subMesh.indexBufferCount = mesh->GetIndexCount();
        mesh->submeshes.push_back(std::move(subMesh));
        mesh->cubeMapTexture = tm.GetTextureCube(tex);
        p.mesh = mesh;
    }
    else
    {
        LOG(Logger::ERROR, "Unable to find texture of CubeSkybox '", p.name, "'.");
    }
}


template <>
void from_json(const nlohmann::json& json, MenuMesh::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));
    if(get_to_if_exists(json, "turn_speed", p.turnSpeed))
    {
        p.turnSpeed *= DirectX::XM_PI / 180.0f;
    }
    if(get_to_if_exists(json, "rotation_axis", p.rotationAxis))
    {
        p.rotationAxis.Normalize();
    }
}

template <>
void from_json(const nlohmann::json& json, Billboard::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    if (std::string billboardType; get_to_if_exists(json, "billboard_type", billboardType))
    {
        if (billboardType == "world")
        {
            p.billboardType = Billboard::WORLD_ALIGNED;
        }
        else if (billboardType == "screen")
        {
            p.billboardType = Billboard::SCREEN_ALIGNED;
        }
        else if (billboardType == "axial")
        {
            p.billboardType = Billboard::AXIAL_ALIGNED;
        }
        else
        {
            LOG(Logger::ERROR, "Billboard ", p.name, " has invalid billboard type: ", p.billboardType);
        }
    }

    if (std::string meshName; get_to_if_exists(json, "mesh", meshName))
    {
        p.mesh = mm.GetAsset<QuadMesh>(meshName);
    }
}

template <>
void from_json(const nlohmann::json& j, Transform& p)
{
    if (j.contains("rotation"))
    {
        auto& rotNode = j.at("rotation");
        const auto rotFactors = rotNode.size();
        if (rotFactors == 3)
        {
            auto eulerRot = rotNode.get<Vector3>();
            auto vecRot = eulerRot * DirectX::XM_PI / 180.0f;
            p.rotation = Quaternion::CreateFromYawPitchRoll(vecRot);
        }
        else { rotNode.get_to(p.rotation); }
    }

    get_to_if_exists(j, "position", p.position);
    get_to_if_exists(j, "scale", p.scale);
}

template <>
void to_json(nlohmann::json& j, const Transform& p)
{
    j["rotation"] = p.rotation;
    j["position"] = p.position;
    j["scale"] = p.scale;
}

template <>
void from_json(const nlohmann::json& json, Sphere::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    if (std::string meshName; get_to_if_exists(json, "mesh", meshName))
    {
        p.mesh = mm.GetAsset<SphereMesh>(meshName);
    }
}

template <>
void from_json(const nlohmann::json& json, Cube::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    if (std::string meshName; get_to_if_exists(json, "mesh", meshName))
    {
        p.mesh = mm.GetAsset<CubeMesh>(meshName);
    }
}

template <>
void from_json(const nlohmann::json& json, Decal::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    if (std::string meshName; get_to_if_exists(json, "mesh", meshName))
    {
        p.mesh = mm.GetAsset<DecalMesh>(meshName);
    }
}

template <>
void from_json(const nlohmann::json& json, InstancedEntity::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));

    const auto instances = json.at("instances").get<std::vector<nlohmann::basic_json<>>>();

    std::vector<Transform> instanceTransform;
    for (auto& jTransform : instances)
    {
        instanceTransform.push_back(jTransform.get<Transform>());
    }
    p.instanceTransforms = instanceTransform;
}

template <>
void from_json(const nlohmann::json& json, Firefly::Params& p)
{
    from_json(json, static_cast<InstancedEntity::Params&>(p));

    Billboard::Params billboardParams;
    nlohmann::json billboardJson;
    get_to_if_exists(json, "billboard", billboardJson);
    from_json(billboardJson, billboardParams);

    get_to_if_exists(json, "color", p.color);
    get_to_if_exists(json, "coefficients", p.coefficients);

    p.billboard = billboardParams;

}

template <>
void from_json(const nlohmann::json& json, TriggerBox::Params& p)
{
    if (json.contains("physics"))
        LOGF(Logger::WARN, "Physics on trigger will be overriden by trigger");

    from_json(json, static_cast<Entity::Params&>(p));
}

template <>
void from_json(const nlohmann::json& json, CheckpointTrigger::Params& p)
{
    from_json(json, static_cast<TriggerBox::Params&>(p));
    get_to_if_exists(json, "checkpoint_index", p.checkpointIndex);
}

template <>
void from_json(const nlohmann::json& json, Vehicle::Params& p)
{
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    if (json.contains("physics"))
        LOGF(Logger::WARN, "Physics on vehicle will be overriden by vehicle physics");
    from_json(json, static_cast<Entity::Params&>(p));

    p.wheelMeshes.fill(nullptr);
    if (std::string meshName; get_to_if_exists(json, "wheelFR", meshName))
    {
        p.wheelMeshes[0] = mm.GetAsset<BaseMesh>(meshName);
    }
    if (std::string meshName; get_to_if_exists(json, "wheelFL", meshName))
    {
        p.wheelMeshes[1] = mm.GetAsset<BaseMesh>(meshName);
    }
    if (std::string meshName; get_to_if_exists(json, "wheelBR", meshName))
    {
        p.wheelMeshes[2] = mm.GetAsset<BaseMesh>(meshName);
    }
    if (std::string meshName; get_to_if_exists(json, "wheelBL", meshName))
    {
        p.wheelMeshes[3] = mm.GetAsset<BaseMesh>(meshName);
    }
    if(std::ranges::any_of(p.wheelMeshes, [](const BaseMesh* baseMesh){return baseMesh == nullptr;}))
    {
        LOG(Logger::FATAL, "One the wheel meshess is nullptr.");
    }
    get_to_if_exists(json, "mesh_physics_offset", p.meshPhysicsOffset);
    get_to_if_exists(json, "wheelFR_physics_offset", p.wheelMeshOffsets[0]);
    get_to_if_exists(json, "wheelFL_physics_offset", p.wheelMeshOffsets[1]);
    get_to_if_exists(json, "wheelBR_physics_offset", p.wheelMeshOffsets[2]);
    get_to_if_exists(json, "wheelBL_physics_offset", p.wheelMeshOffsets[3]);


}

template <>
void from_json(const nlohmann::json& json, Door::Params& p)
{
    from_json(json, static_cast<Entity::Params&>(p));
}

template <>
void from_json(const nlohmann::json& j, DirectionalLight& p)
{
    get_to_if_exists(j, "color", p.Color);
    get_to_if_exists(j, "direction", p.Direction);
    get_to_if_exists(j, "casts_shadows", p.castsShadows);
    get_to_if_exists(j, "is_active", p.isActive);
}

template <>
void from_json(const nlohmann::json& j, PointLight& p)
{
    get_to_if_exists(j, "color", p.Color);
    get_to_if_exists(j, "position", p.Position);
    get_to_if_exists(j, "coefficients", p.Coefficients);
    get_to_if_exists(j, "is_active", p.isActive);
}

template <>
void from_json(const nlohmann::json& j, SpotLight& p)
{
    get_to_if_exists(j, "color", p.Color);
    get_to_if_exists(j, "position", p.Position);
    get_to_if_exists(j, "coefficients", p.Coefficients);
    get_to_if_exists(j, "direction", p.Direction);
    get_to_if_exists(j, "inner_cone_angle", p.InnerConeAngle);
    get_to_if_exists(j, "outer_cone_angle", p.OuterConeAngle);
    get_to_if_exists(j, "is_active", p.isActive);
}
}
