#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_set>

#include "DataStructures/FixedVector.h"
#include "Entities/GrassGenerator.h"
#include "Rendering/Lights/PointLight.h"
#include "Rendering/Lights/DirectionalLight.h"
#include "Rendering/Lights/SpotLight.h"
#include "Util/JsonUtil.h"

namespace Snail
{
class Decal;
class CubeSkybox;
class Entity;

struct SceneData
{
    std::string sourceFilename;

    static constexpr uint8_t MAX_DIR_LIGHTS = 2;
    FixedVector<DirectionalLight, MAX_DIR_LIGHTS> directionalLights;

    static constexpr uint8_t MAX_SPOT_LIGHTS = 10;
    FixedVector<SpotLight, MAX_SPOT_LIGHTS> spotLights;

    static constexpr uint8_t MAX_PT_LIGHTS = 40;
    FixedVector<PointLight, MAX_PT_LIGHTS> pointLights;

    std::vector<std::unique_ptr<Entity>> objects;
    std::unordered_set<Entity*> objectsToRemove;

    std::unique_ptr<CubeSkybox> skybox;

    std::vector<std::unique_ptr<Decal>> decals;

    std::vector<std::unique_ptr<GrassGenerator>> grassPatches;

    SceneData() = default;
    SceneData(SceneData&&) = default;
    SceneData& operator=(SceneData&&) = default;
    void Clear();
};

class SceneParser
{
    SceneData data;
    std::mutex objectsMutex;

    void ParseGrass(const nlohmann::basic_json<>& grassJson);
    void ParseMesh(const nlohmann::basic_json<>& jMesh);
    void ParseCamera(const nlohmann::json& cameraJson);
    void ParseScene(const nlohmann::json& sceneJson);
    SceneParser() = default;

public:
    SceneParser(const SceneParser&) = delete;
    SceneParser& operator=(const SceneParser&) = delete;
    SceneParser(SceneParser&&) = delete;
    SceneParser& operator=(SceneParser&&) = delete;

    static std::optional<SceneData> Parse(const std::string& filename, const std::atomic_bool& shouldStopLoading);
    static std::unique_ptr<Entity> ParseEntity(const nlohmann::json& object);
    std::unique_ptr<Entity> ParseEntityObject(const nlohmann::json& object);
};

}
