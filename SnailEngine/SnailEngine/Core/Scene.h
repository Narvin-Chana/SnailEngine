#pragma once
#include <vector>

#include "RendererModule.h"
#include "SceneParser.h"
#include "Rendering/Lights/DirectionalLight.h"
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/UI/LoadingScreen.h"
#include "Rendering/DrawContext.h"

#include "Rendering/UI/MainMenu.h"

namespace Snail
{

class Entity;
class Sprite;
class DrawContext;

class Scene
{
    static constexpr const char* DEFAULT_SCENE_PATH = "Resources/Scenes/MainMenu.json";

    SceneData data;

    std::thread loadingThread;

	D3D11Buffer directionalLightsBuffer;
	D3D11Buffer spotLightsBuffer;
	D3D11Buffer pointLightsBuffer;
	D3D11Buffer sceneInfoBuffer;

	mutable struct DX_ALIGN SceneBufferData
	{
        DX_ALIGN Matrix view;
        DX_ALIGN Matrix invView;
        DX_ALIGN Matrix invViewProj;
		DX_ALIGN Vector3 cameraPosition;
		unsigned int nbDirectional;
		unsigned int nbSpotLight;
		unsigned int nbPointLight;
        float volumetricDensityFactor = 0.2f;
	} sceneInfo{};

    std::optional<std::string> loadOnNextFrame = {};
    bool isLoading = false;
    std::atomic<bool> shouldStopLoading = false;

    std::vector<std::unique_ptr<UIElement>> sceneUiElements;
    std::unique_ptr<MainMenu> mainMenuUI = std::make_unique<MainMenu>();
    void StartLoadFromFile(const std::string& filename);

public:
    Scene(const std::string& scenePath = DEFAULT_SCENE_PATH);

    Scene(const Scene& scene) = delete;
    Scene operator=(const Scene& scene) = delete;

	~Scene();

    auto GetVolumetricFactor() const { return sceneInfo.volumetricDensityFactor; }
    void SetVolumetricFactor(float val);

	const auto& GetDirectionalLights() const { return data.directionalLights; }
    const auto& GetSpotLights() const { return data.spotLights; }
	const auto& GetPointLights() const { return data.pointLights; }

    std::vector<Entity*> GetEntities() const;
    std::vector<GrassGenerator*> GetGrassPatches() const;
    std::vector<Decal*> GetDecals() const;

    const D3D11Buffer& GetDirectionalLightsBuffer();
	const D3D11Buffer& GetSpotLightsBuffer();
	const D3D11Buffer& GetPointLightsBuffer();
	const D3D11Buffer& GetSceneInfoBuffer();

    void LoadScene(const std::string& name);
    void DoneLoading();
    bool IsLoading();

    void RemoveEntity(Entity* entityToRemove);
    void CleanupRemoveEntity();
	void Update(float dt);
	void Draw(DrawContext& ctx) const;
	void DrawDecals(DrawContext& ctx) const;
    void DrawSkybox(DrawContext& ctx) const;
	void Cleanup();
    void Reset();
    void ReloadShaders() const;
    void RenderImGui();
    void RenderGrassImGui();
};

class SceneDrawContext : public DrawContext
{
    const DirectX::BoundingFrustum& cameraFrustum;

public:
    SceneDrawContext(RendererModule* renderer, D3D11Device* device, const DirectX::BoundingFrustum& cam);

    bool ShouldBeCulled(const DirectX::BoundingBox&) override;
    bool ShouldBeCulled(const DirectX::BoundingOrientedBox&) override;
    bool ShouldBeCulled(const DirectX::BoundingSphere&) override;
};

}
