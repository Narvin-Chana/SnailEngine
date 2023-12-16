#include "stdafx.h"
#include "SceneParser.h"

#include "ThreadPool.h"
#include "WindowsEngine.h"
#include "Entities/Billboard.h"
#include "Entities/CubeSkybox.h"
#include "Entities/Terrain.h"
#include "Entities/Sphere.h"
#include "Entities/Vehicle.h"
#include "Entities/Cube.h"
#include "Entities/Decal.h"
#include "Entities/InstancedEntity.h"
#include "Entities/Firefly.h"
#include "Entities/InvisibleWall.h"
#include "Entities/Triggers/AdaptiveLightingTrigger.h"
#include "Entities/Triggers/BoostTrigger.h"
#include "Entities/Triggers/CheckpointTrigger.h"
#include "Entities/Triggers/KeyTrigger.h"
#include "Entities/MenuMesh.h"
#include "Mesh/BillboardMesh.h"
#include "Mesh/CubeMesh.h"
#include "Mesh/DecalMesh.h"
#include "Mesh/QuadMesh.h"
#include "Mesh/SphereMesh.h"
#include "Util/DebugUtil.h"

namespace Snail
{
void SceneData::Clear()
{
    directionalLights.clear();
    spotLights.clear();
    pointLights.clear();
    objects.clear();
    grassPatches.clear();
    decals.clear();
    skybox.reset();
    LOG("Cleared scene data");
}

void SceneParser::ParseGrass(const nlohmann::basic_json<>& grassJson)
{
    float grassDensity;
    if (!get_to_if_exists(grassJson, "grass_density", grassDensity))
    {
        LOG(Logger::FATAL, "The patch of grass must have a clearly defined density.");
    }
    std::array<uint32_t, 2> regionCount;
    if (!get_to_if_exists(grassJson, "region_count", regionCount))
    {
        LOG(Logger::FATAL, "The patch of grass must have a clearly defined regionCount.");
    }
    Transform grassPatchPosition;
    if (!get_to_if_exists(grassJson, "transform", grassPatchPosition))
    {
        LOG(Logger::FATAL, "The patch of grass must have a clearly defined transform.");
    }
    std::string samplePatchTextureFilepath;

    Texture2D* tex = nullptr;
    get_to_if_exists(grassJson, "sample_filepath", samplePatchTextureFilepath);
    if (!samplePatchTextureFilepath.empty())
    {
        tex = WindowsEngine::GetInstance().GetModule<TextureManager>().GetTexture2D(samplePatchTextureFilepath);
    }

    std::lock_guard lock{DeviceMutex};
    data.grassPatches.push_back(std::make_unique<GrassGenerator>(grassDensity, regionCount, grassPatchPosition, tex));
}

void SceneParser::ParseMesh(const nlohmann::basic_json<>& jMesh)
{
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    using namespace nlohmann;
    const std::string type = jMesh.at("type").get<std::string>();
    const std::string meshName = jMesh.at("name").get<std::string>();

    BaseMesh* mesh{};
    if (type == "cubemesh")
    {
        std::unique_ptr<CubeMesh> cubeMesh = std::make_unique<CubeMesh>();

        if (TexturedMaterial mat{}; get_to_if_exists(jMesh, "material", mat))
        {
            assert(!cubeMesh->submeshes.empty());
            cubeMesh->submeshes[0].SetMaterial(mat);
        }

        mesh = mm.SaveAsset<CubeMesh>(meshName, std::move(cubeMesh));
    }
    else if (type == "quadmesh")
    {
        std::unique_ptr<QuadMesh> cubeMesh = std::make_unique<QuadMesh>();

        if (TexturedMaterial mat{}; get_to_if_exists(jMesh, "material", mat))
        {
            assert(!cubeMesh->submeshes.empty());
            cubeMesh->submeshes[0].SetMaterial(mat);
        }

        mesh = mm.SaveAsset<QuadMesh>(meshName, std::move(cubeMesh));
    }
    else if (type == "billboardmesh")
    {
        std::unique_ptr<BillboardMesh> quadMesh = std::make_unique<BillboardMesh>();
        static TextureManager& textureManager = WindowsEngine::GetModule<TextureManager>();

        if (std::string quadTexture; get_to_if_exists(jMesh, "texture", quadTexture))
        {
            quadMesh->quadTexture = textureManager.GetTexture2D(quadTexture);
            assert(quadMesh->quadTexture);
        }
        else
        {
            quadMesh->quadTexture = textureManager.GetTexture2D(TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME);
        }

        mesh = mm.SaveAsset<BillboardMesh>(jMesh.at("name").get<std::string>(), std::move(quadMesh));
    }
    else if (type == "spheremesh")
    {
        int stacks = 20, slices = 20;

        get_to_if_exists(jMesh, "stacks", stacks);
        get_to_if_exists(jMesh, "slices", slices);

        std::unique_ptr<SphereMesh> sphereMesh = std::make_unique<SphereMesh>(stacks, slices);

        if (TexturedMaterial mat{}; get_to_if_exists(jMesh, "material", mat))
        {
            assert(!sphereMesh->submeshes.empty());
            sphereMesh->submeshes[0].SetMaterial(mat);
        }

        mesh = mm.SaveAsset<SphereMesh>(meshName, std::move(sphereMesh));
    }
    else if (type == "decalmesh")
    {
        std::unique_ptr<DecalMesh> decalMesh = std::make_unique<DecalMesh>();

        if (std::string diffuseFile; get_to_if_exists(jMesh, "diffuse_filepath", diffuseFile))
        {
            tm.SaveAsset<Texture2D>(diffuseFile);
            decalMesh->SetAllMaterialMember(diffuseFile, &TexturedMaterial::diffuseTexture);
        }

        mesh = mm.SaveAsset<DecalMesh>(meshName, std::move(decalMesh));
    }
    else if (type == "mesh")
    {
        const std::string meshFile = jMesh.at("filepath").get<std::string>();
        if (!std::filesystem::exists(meshFile))
        {
            LOGF(Logger::ERROR, "Mesh source file not found. path: \"{}\"", meshFile);
            return;
        }

        Mesh<uint32_t>* complexMesh = mm.SaveAsset<Mesh<uint32_t>>(meshName, meshFile);
        mesh = complexMesh;

        // If a texture is passed, override all submesh's materials
        if (std::string diffuseFile; get_to_if_exists(jMesh, "diffuse_filepath", diffuseFile))
        {
            tm.SaveAsset<Texture2D>(diffuseFile);
            complexMesh->SetAllMaterialMember(diffuseFile, &TexturedMaterial::diffuseTexture);
        }

        if (std::string blendDiffuseFile; get_to_if_exists(jMesh, "primary_blend_diffuse_filepath", blendDiffuseFile))
        {
            tm.SaveAsset<Texture2D>(blendDiffuseFile);
            complexMesh->SetAllMaterialMember(blendDiffuseFile, &TexturedMaterial::primaryBlendDiffuseTexture);
        }

        if (std::string blendFile; get_to_if_exists(jMesh, "primary_blend_filepath", blendFile))
        {
            tm.SaveAsset<Texture2D>(blendFile);
            complexMesh->SetAllMaterialMember(blendFile, &TexturedMaterial::primaryBlendTexture);
        }

        if (std::string blendDiffuseFile; get_to_if_exists(jMesh, "secondary_blend_diffuse_filepath", blendDiffuseFile))
        {
            tm.SaveAsset<Texture2D>(blendDiffuseFile);
            complexMesh->SetAllMaterialMember(blendDiffuseFile, &TexturedMaterial::secondaryBlendDiffuseTexture);
        }

        if (std::string blendFile; get_to_if_exists(jMesh, "secondary_blend_filepath", blendFile))
        {
            tm.SaveAsset<Texture2D>(blendFile);
            complexMesh->SetAllMaterialMember(blendFile, &TexturedMaterial::secondaryBlendTexture);
        }

        if (std::string ambientFilepath; get_to_if_exists(jMesh, "ambient_filepath", ambientFilepath))
        {
            tm.SaveAsset<Texture2D>(ambientFilepath);
            complexMesh->SetAllMaterialMember(ambientFilepath, &TexturedMaterial::ambientTexture);
        }

        if (std::string specularFilepath; get_to_if_exists(jMesh, "specular_filepath", specularFilepath))
        {
            tm.SaveAsset<Texture2D>(specularFilepath);
            complexMesh->SetAllMaterialMember(specularFilepath, &TexturedMaterial::specularTexture);
        }

        if (std::string normalMapFilepath; get_to_if_exists(jMesh, "normalmap_filepath", normalMapFilepath))
        {
            tm.SaveAsset<Texture2D>(normalMapFilepath);
            complexMesh->SetAllMaterialMember(normalMapFilepath, &TexturedMaterial::normalMapTexture);
        }
    }

    if (std::string culling; get_to_if_exists(jMesh, "culling", culling))
    {
        if (culling == "front")
            mesh->SetCullingType(BaseMesh::CullingType::FRONT);
        else if (culling == "back")
            mesh->SetCullingType(BaseMesh::CullingType::BACK);
        else if (culling == "no")
            mesh->SetCullingType(BaseMesh::CullingType::NO);
        else
            LOGF("Invalid value for mesh culling : {}", culling);
    }

    if (bool isTranslucent; get_to_if_exists(jMesh, "translucent", isTranslucent))
    {
        mesh->SetTranslucent(isTranslucent);
    }
}

void SceneParser::ParseCamera(const nlohmann::json& cameraJson)
{
    static CameraManager& cm = WindowsEngine::GetModule<CameraManager>();

    std::string type;
    get_to_if_exists(cameraJson, "type", type);
    if (type == "orthographic")
    {
        Transform t;
        get_to_if_exists(cameraJson, "transform", t);

        cm.AddCameraOrtho(t);
    }
    else
    {
        float FOV = PerspectiveProjection::DEFAULT_FOV;
        if (get_to_if_exists(cameraJson, "FOV", FOV)) { FOV *= DirectX::XM_PI / 180.0f; }

        Transform t;
        get_to_if_exists(cameraJson, "transform", t);

        cm.AddCameraPerspective(t, FOV);
    }
}

std::unique_ptr<Entity> SceneParser::ParseEntity(const nlohmann::json& object)
{
    std::unique_ptr<Entity> entity;

    if (const std::string type = object.at("type").get<std::string>(); type == "sphere")
    {
        entity = Entity::CreateObject<Sphere>(object);
    }
    else if (type == "cube")
    {
        entity = Entity::CreateObject<Cube>(object);
    }
    else if (type == "checkpoint_trigger")
    {
        entity = Entity::CreateObject<CheckpointTrigger>(object);
    }
    else if (type == "boost_trigger")
    {
        entity = Entity::CreateObject<BoostTrigger>(object);
    }
    else if (type == "key_trigger")
    {
        entity = Entity::CreateObject<KeyTrigger>(object);
    }
    else if (type == "lighting_trigger")
    {
        entity = Entity::CreateObject<AdaptiveLightingTrigger>(object);
    }
    else if (type == "billboard")
    {
        entity = Entity::CreateObject<Billboard>(object);
    }
    else if (type == "terrain")
    {
        entity = Entity::CreateObject<Terrain>(object);
    }
    else if (type == "vehicle")
    {
        entity = Entity::CreateObject<Vehicle>(object);
    }
    else if (type == "door")
    {
        entity = Entity::CreateObject<Door>(object);
    }
    else if (type == "entity")
    {
        entity = Entity::CreateObject<Entity>(object);
    }
    else if (type == "instanced_entity")
    {
        entity = Entity::CreateObject<InstancedEntity>(object);
    }
    else if(type == "firefly")
    {
        entity = Entity::CreateObject<Firefly>(object);
    }
    else if (type == "invisible_wall")
    {
        entity = Entity::CreateObject<InvisibleWall>(object);
    }
    else if (type == "menu_mesh")
    {
        entity = Entity::CreateObject<MenuMesh>(object);
    }
    return entity;
}

std::unique_ptr<Entity> SceneParser::ParseEntityObject(const nlohmann::json& object)
{
    std::unique_ptr<Entity> entity;

    if (const std::string type = object.at("type").get<std::string>(); type == "sphere")
    {
        entity = Entity::CreateObject<Sphere>(object);
    }
    else if (type == "cube")
    {
        entity = Entity::CreateObject<Cube>(object);
    }
    else if (type == "checkpoint_trigger")
    {
        entity = Entity::CreateObject<CheckpointTrigger>(object);
    }
    else if (type == "boost_trigger")
    {
        entity = Entity::CreateObject<BoostTrigger>(object);
    }
    else if (type == "key_trigger")
    {
        entity = Entity::CreateObject<KeyTrigger>(object);
    }
    else if (type == "lighting_trigger")
    {
        entity = Entity::CreateObject<AdaptiveLightingTrigger>(object);
    }
    else if (type == "billboard")
    {
        entity = Entity::CreateObject<Billboard>(object);
    }
    else if (type == "terrain")
    {
        entity = Entity::CreateObject<Terrain>(object);
    }
    else if (type == "vehicle")
    {
        entity = Entity::CreateObject<Vehicle>(object);
    }
    else if (type == "door")
    {
        entity = Entity::CreateObject<Door>(object);
    }
    else if (type == "entity")
    {
        entity = Entity::CreateObject<Entity>(object);
    }
    else if (type == "instanced_entity")
    {
        entity = Entity::CreateObject<InstancedEntity>(object);
    }
    else if (type == "firefly")
    {
        entity = Entity::CreateObject<Firefly>(object);
        reinterpret_cast<Firefly*>(entity.get())->addLights(&data);
    }
    else if (type == "invisible_wall")
    {
        entity = Entity::CreateObject<InvisibleWall>(object);
    }
    else if (type == "menu_mesh")
    {
        entity = Entity::CreateObject<MenuMesh>(object);
    }
    return entity;
}

void SceneParser::ParseScene(const nlohmann::json& sceneJson)
{
    if (Color color; get_to_if_exists(sceneJson, "clear_color", color))
    {
        WindowsEngine::GetInstance().GetRenderDevice()->SetClearColor(color);
    }

    bool is_main_menu = false; get_to_if_exists(sceneJson, "is_main_menu", is_main_menu);
    WindowsEngine::GetInstance().isMainMenuLoaded = is_main_menu;

    if(int numberOfLaps; get_to_if_exists(sceneJson, "number_of_laps", numberOfLaps))
    {
        WindowsEngine::GetInstance().GetModule<GameManager>().SetNumberOfLaps(numberOfLaps);
    }
    else{
        WindowsEngine::GetInstance().GetModule<GameManager>().SetNumberOfLaps(3);
    }

    const auto lights = sceneJson.at("lights").get<std::vector<nlohmann::basic_json<>>>();
    for (const auto& light : lights)
    {
        const std::string type = light.at("type").get<std::string>();
        if (type == "directional")
            data.directionalLights.push_back(light.get<DirectionalLight>());
        if (type == "point")
            data.pointLights.push_back(light.get<PointLight>());
        if (type == "spot")
            data.spotLights.push_back(light.get<SpotLight>());
    }

    if (sceneJson.contains("skybox"))
    {
        data.skybox = Entity::CreateObject<CubeSkybox>(sceneJson.at("skybox"));
    }
}

std::optional<SceneData> SceneParser::Parse(const std::string& filename, const std::atomic_bool& shouldStopLoading)
{
    static CameraManager& cm = WindowsEngine::GetModule<CameraManager>();

    SceneParser sceneData;
    sceneData.data.sourceFilename = filename;
    ThreadPool pool;
    std::vector<ThreadPool::TaskHandle> handles;

    auto before = std::chrono::high_resolution_clock::now();

    using namespace nlohmann;

    std::ifstream file{filename};
    const json sceneJson = json::parse(file, nullptr, true, true);

    sceneData.ParseScene(sceneJson);

    // Parse cameras
    if (std::vector<basic_json<>> camerasJson; get_to_if_exists(sceneJson, "cameras", camerasJson))
    {
        for (const auto& cameraJson : camerasJson)
        {
            try
            {
                if (shouldStopLoading)
                    return {};

                sceneData.ParseCamera(cameraJson);
            }
            catch (detail::exception e)
            {
                LOG(Logger::ERROR, "Unable to load json camera: ", e.what());
            }
        }
    }
    else
    {
        // Add default cam if none were found in the json
        cm.AddCameraPerspective({});
    }

    // Parse grass patches
    if (sceneJson.contains("grass_patches"))
    {
        const std::vector<basic_json<>> jsonGrassPatches = sceneJson.at("grass_patches").get<std::vector<basic_json<>>>();
        for (const auto& grass : jsonGrassPatches)
        {
            // Don't multi-thread since almost all grass generation happens on the GPU side.
            try
            {
                if (shouldStopLoading)
                    return {};

                sceneData.ParseGrass(grass);
            }
            catch (detail::exception e)
            {
                LOG(Logger::ERROR, "Unable to load json grass: ", e.what());
            }
        }
    }

    // Parse meshes
    const std::vector<basic_json<>> jsonMeshes = sceneJson.at("meshes").get<std::vector<basic_json<>>>();
    for (const auto& mesh : jsonMeshes)
    {
        handles.push_back(pool.AddWaitableTask([&]
        {
            try
            {
                if (shouldStopLoading)
                    return;

                sceneData.ParseMesh(mesh);
            }
            catch (detail::exception e)
            {
                LOG(Logger::ERROR, "Unable to load json mesh: ", e.what());
            }
        }));
    }

    for (auto& handle : handles)
        pool.WaitFor(handle);
    handles.clear();

    if (shouldStopLoading)
        return {};

    cm.ChangeCamera(0);

    // Parse entities
    const auto jsonObjects = sceneJson.at("objects").get<std::vector<basic_json<>>>();
    for (const auto& object : jsonObjects)
    {
        handles.push_back(pool.AddWaitableTask([&]
        {
            try
            {
                if (shouldStopLoading)
                    return;

                std::unique_ptr<Entity> ptr = sceneData.ParseEntityObject(object);

                std::lock_guard lock{sceneData.objectsMutex};
                sceneData.data.objects.push_back(std::move(ptr));
            }
            catch (detail::exception e)
            {
                LOG(Logger::ERROR, "Unable to load json entity: ", e.what());
            }
        }));
    }

    for (auto& handle : handles)
        pool.WaitFor(handle);
    handles.clear();

    // Parse decals
    if (sceneJson.contains("decals"))
    {
        const auto jsonDecals = sceneJson.at("decals").get<std::vector<basic_json<>>>();
        for (const auto& decal : jsonDecals)
        {
            std::unique_ptr<Decal> decalPtr = Entity::CreateObject<Decal>(decal);
            sceneData.data.decals.push_back(std::move(decalPtr));
        }
    }

    auto after = std::chrono::high_resolution_clock::now();
    DebugPrint("Loading done in: ", std::chrono::duration_cast<std::chrono::milliseconds>(after - before), '\n');

    return std::move(sceneData.data);
}
}
