#include "stdafx.h"

#include "Scene.h"

#include <memory>
#include <random>

#include "SceneParser.h"
#include "ThreadPool.h"
#include "Core/WindowsEngine.h"
#include "Core/Math/Transform.h"
#include "Core/Camera/CameraManager.h"
#include "Rendering/UI/Button.h"
#include "Entities/Entity.h"
#include "Entities/Decal.h"
#include "Entities/CubeSkybox.h"

using namespace physx;

namespace Snail
{
Scene::Scene(const std::string& scenePath)
    : directionalLightsBuffer{D3D11Buffer::CreateConstantBuffer<DirectionalLight[SceneData::MAX_DIR_LIGHTS]>()}
    , spotLightsBuffer{D3D11Buffer::CreateConstantBuffer<SpotLight[SceneData::MAX_SPOT_LIGHTS]>()}
    , pointLightsBuffer{D3D11Buffer::CreateConstantBuffer<PointLight[SceneData::MAX_PT_LIGHTS]>()}
    , sceneInfoBuffer{D3D11Buffer::CreateConstantBuffer<SceneBufferData>()}
{
    StartLoadFromFile(scenePath);
}

Scene::~Scene()
{
    if (loadingThread.joinable())
        loadingThread.join();
}
void Scene::SetVolumetricFactor(float val)
{
    sceneInfo.volumetricDensityFactor = val;
}

void Scene::RemoveEntity(Entity* entityToRemove)
{
    data.objectsToRemove.insert(entityToRemove);
}

void Scene::CleanupRemoveEntity()
{
    if (data.objectsToRemove.empty())
        return;

    for (auto entityToRemove : data.objectsToRemove)
    {
        const auto it = std::ranges::find_if(data.objects,
            [entityToRemove](const std::unique_ptr<Entity>& entity)
            {
                return entityToRemove == entity.get();
            });

        data.objects.erase(it);
    }
    data.objectsToRemove.clear();
}

void Scene::Update(const float dt)
{
    for (const std::unique_ptr<UIElement>& elem : sceneUiElements)
        elem->Update(dt);

    for (const std::unique_ptr<Entity>& entity : data.objects)
        entity->Update(dt);

    for (const std::unique_ptr<GrassGenerator>& grassGenerator : data.grassPatches)
        grassGenerator->Update(dt);

    if (WindowsEngine::GetInstance().isMainMenuLoaded)
        mainMenuUI->Update(dt);
}

const D3D11Buffer& Scene::GetSceneInfoBuffer()
{
    const Camera* camera = WindowsEngine::GetInstance().GetCamera();
    sceneInfo.invViewProj = camera->GetViewProjectionMatrix().Invert().Transpose();
    sceneInfo.invView = camera->GetViewMatrix().Invert().Transpose();
    sceneInfo.view = camera->GetViewMatrix().Transpose();
    sceneInfo.cameraPosition = camera->GetWorldTransform().position;
    sceneInfo.nbDirectional = static_cast<int>(data.directionalLights.size());
    sceneInfo.nbSpotLight = static_cast<int>(data.spotLights.size());
    sceneInfo.nbPointLight = static_cast<int>(data.pointLights.size());

    sceneInfoBuffer.UpdateData(&sceneInfo, sizeof(sceneInfo));

    return sceneInfoBuffer;
}

void Scene::LoadScene(const std::string& name)
{
    loadOnNextFrame = name;
}

void Scene::StartLoadFromFile(const std::string& filename)
{
    if (isLoading)
    {
        LOGF(Logger::WARN, "Tried to load a scene named \"{}\" when already loading another", filename);
        return;
    }

    LOGF("Loading scene \"{}\"", filename);
    Cleanup();

#ifdef _DEBUG
    static auto dev = WindowsEngine::GetInstance().GetRenderDevice();
    if (dev->debugDev)
        dev->debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif

    isLoading = true;
    static auto& gameManager = WindowsEngine::GetModule<GameManager>();
    gameManager.Cleanup();

    loadingThread = std::thread([=]
    {
        try
        {
            if (auto parsed = SceneParser::Parse(filename, shouldStopLoading); parsed.has_value())
            {
                data = std::move(*parsed);
                LOGF("Done loading scene:\n" "\tDirectional light count: {}\n" "\tPoint light count: {}\n" "\tSpot light count: {}\n"
                    "\tObject count: {}",
                    data.directionalLights.size(),
                    data.pointLights.size(),
                    data.spotLights.size(),
                    data.objects.size());

                gameManager.Start();
            }
            else { LOG(Logger::WARN, "Scene parsing interrupted"); }
        }
        catch (nlohmann::detail::exception& e)
        {
            constexpr int bufferSize = 1024;
            wchar_t message[bufferSize];

            const char* msg = e.what();
            size_t numCharacterConverted;
            mbstowcs_s(&numCharacterConverted, message, msg, bufferSize - 1);

            ::MessageBox(nullptr, message, L"JSON parse error", MB_ICONERROR);

            // We want to crash here
            throw;
        } catch (SnailException& e)
        {
            constexpr int bufferSize = 128;
            wchar_t message[bufferSize];

            const char* msg = e.what();
            size_t numCharacterConverted;
            mbstowcs_s(&numCharacterConverted, message, msg, bufferSize - 1);

            size_t numCharacterConvertedType;
            wchar_t type[bufferSize];
            mbstowcs_s(&numCharacterConvertedType, type, e.GetType(), bufferSize - 1);

            ::MessageBox(nullptr, message, type, MB_ICONERROR);

            // We want to crash here
            throw;
        }

        if(WindowsEngine::GetInstance().isMainMenuLoaded)
        {
            WindowsEngine::GetCamera()->AddOverlay("mainMenu",mainMenuUI.get());
            mainMenuUI->SetActive(true);
        }
        else
        {
            WindowsEngine::GetCamera()->RemoveOverlay("mainMenu");
        }
        
        DoneLoading();
    });
}

void Scene::DoneLoading()
{
    static auto& engine = WindowsEngine::GetInstance();
    engine.ResetClock();
    isLoading = false;
}

bool Scene::IsLoading()
{
    if (loadOnNextFrame.has_value())
    {
        StartLoadFromFile(loadOnNextFrame.value());
        loadOnNextFrame = {};
    }
    return isLoading;
}

std::vector<Entity*> Scene::GetEntities() const
{
    auto retVal = data.objects | std::views::transform([](auto& ptr) { return ptr.get(); });
    return {retVal.begin(), retVal.end()};
}

std::vector<GrassGenerator*> Scene::GetGrassPatches() const
{
    auto retVal = data.grassPatches | std::views::transform([](auto& ptr) { return ptr.get(); });
    return {retVal.begin(), retVal.end()};
}

std::vector<Decal*> Scene::GetDecals() const
{
    auto retVal = data.decals | std::views::transform([](auto& ptr) { return ptr.get(); });
    return {retVal.begin(), retVal.end()};
}

const D3D11Buffer& Scene::GetDirectionalLightsBuffer()
{
    directionalLightsBuffer.UpdateData(data.directionalLights);
    return directionalLightsBuffer;
}

const D3D11Buffer& Scene::GetPointLightsBuffer()
{
    pointLightsBuffer.UpdateData(data.pointLights);
    return pointLightsBuffer;
}

const D3D11Buffer& Scene::GetSpotLightsBuffer()
{
    spotLightsBuffer.UpdateData(data.spotLights);
    return spotLightsBuffer;
}

void Scene::Draw(DrawContext& ctx) const
{
    for (Entity* entity : GetEntities())
        entity->Draw(ctx);
    
    ctx.device->SetNoCulling();
    for (GrassGenerator* grassPatch : GetGrassPatches())
        grassPatch->Draw(ctx);
}

void Scene::DrawDecals(DrawContext& ctx) const
{
    for (Decal* decal : GetDecals())
    {
        // Could flip face culling and disable depth test if the camera is inside the decal's box but this scenario should almost never happen
        decal->Draw(ctx);
    }
}

void Scene::DrawSkybox(DrawContext& ctx) const
{
    if(data.skybox)
    data.skybox->Draw(ctx);
}

void Scene::RenderImGui()
{
#ifdef _IMGUI_

    // Create Scene window
    ImGui::Begin("Scene");

    // Scene loading should be moved to the future scene manager according to task #25
    ImGui::Text("Select a scene: ");

    const std::filesystem::path scenesRoot = "Resources/Scenes/";
    const std::string sceneExtension = ".json";

    static std::filesystem::path currentlySelectedScenePath;

    static int currentlySelected = -1;

    if (ImGui::BeginCombo("##select combobox scene", currentlySelectedScenePath.filename().string().c_str()))
    {
        int i = 0;
        for (std::filesystem::directory_entry item : std::filesystem::directory_iterator{scenesRoot})
        {
            std::filesystem::path itemPath = item.path();
            std::string filename = itemPath.filename().string();

            if (item.is_directory() || filename.find(sceneExtension) == std::string::npos)
                continue;

            // remove .json
            filename.resize(filename.size() - sceneExtension.size());

            if (ImGui::Selectable(filename.c_str(), currentlySelected == i))
            {
                currentlySelected = i;
                currentlySelectedScenePath = itemPath;
            }
            i++;
        }

        ImGui::EndCombo();
    }

    if (!currentlySelectedScenePath.string().empty() && ImGui::Button(("Load Scene: " + currentlySelectedScenePath.filename().string()).c_str()))
    {
        LoadScene(currentlySelectedScenePath.string());
    }

    ImGui::SeparatorText(("Scene Entities: " + std::to_string(data.objects.size())).c_str());

    constexpr int MAX_JSON_SIZE = 1024;
    static char text[MAX_JSON_SIZE]{};
    ImGui::InputTextMultiline("##EntityJSON", text, MAX_JSON_SIZE);
    if (ImGui::Button("AddEntity"))
    {
        try
        {
            const nlohmann::json json = nlohmann::json::parse(text);
            data.objects.push_back(SceneParser::ParseEntity(json));
            LOGF("Added entity: {}", text);
        }
        catch (nlohmann::detail::exception& e)
        {
            LOGF("Add entity JSON parse error: {}", e.what());
        }
    }

    if (data.skybox)
    // ImGui Scene Entity;
    if (ImGui::CollapsingHeader((std::to_string(0) + ": " + data.skybox->entityName).c_str(), ImGuiTreeNodeFlags_Framed))
    {
        data.skybox->RenderImGui(0);
    }

    const std::vector<Entity*> entities = GetEntities();
    for (int i = 0; i < static_cast<int>(data.objects.size()); ++i)
    {
        if (Entity* entity = entities[i]; ImGui::CollapsingHeader((std::to_string(i + 1) + ": " + entity->entityName).c_str(), ImGuiTreeNodeFlags_Framed))
        {
            entity->RenderImGui(i);
        }
    }

    ImGui::SeparatorText(("Scene Decals: " + std::to_string(data.decals.size())).c_str());

    const std::vector<Decal*> decals = GetDecals();
    for (int i = 0; i < static_cast<int>(data.decals.size()); ++i)
    {
        if (Decal* decal = decals[i]; ImGui::CollapsingHeader((std::to_string(i) + ": " + decal->entityName).c_str(), ImGuiTreeNodeFlags_Framed))
        {
            decal->RenderImGui(i);
        }
    }

    ImGui::SeparatorText(("Scene Lights: " + std::to_string(data.directionalLights.size())).c_str());

    ImGui::Text("Volumetric Density Factor: ");
    ImGui::DragFloat("##scene volumetric density factor float", &sceneInfo.volumetricDensityFactor, 0.05f, 0.0f, 1.0f);

    // ImGui Scene Entity: 
    for (int i = 0; i < static_cast<int>(data.directionalLights.size()); ++i)
    {
        if (DirectionalLight& directionalLight = data.directionalLights[i]; ImGui::CollapsingHeader(
            (std::to_string(i) + ": Directional Light").c_str(),
            ImGuiTreeNodeFlags_Framed))
        {
            directionalLight.RenderImGui(i);
        }
    }

    for (int i = 0; i < static_cast<int>(data.spotLights.size()); ++i)
    {
        if (SpotLight& spotLight = data.spotLights[i]; ImGui::CollapsingHeader((std::to_string(i) + ": Spot Light").c_str(),
            ImGuiTreeNodeFlags_Framed))
        {
            spotLight.RenderImGui(i);
        }
    }

    for (int i = 0; i < static_cast<int>(data.pointLights.size()); ++i)
    {
        if (PointLight& pointLight = data.pointLights[i]; ImGui::CollapsingHeader((std::to_string(i) + ": Point Light").c_str(),
            ImGuiTreeNodeFlags_Framed))
        {
            pointLight.RenderImGui(i);
        }
    }

    ImGui::End();
#endif
}

void Scene::RenderGrassImGui()
{
#ifdef _IMGUI_
    if (ImGui::CollapsingHeader("Grass Renderer"))
    {
        for (int i = 0; i < static_cast<int>(data.grassPatches.size()); ++i)
        {
            if (ImGui::TreeNode(("Patch " + std::to_string(i)).c_str()))
            {
                ImGui::PushID(i);
                data.grassPatches[i]->RenderImGui();
                ImGui::PopID();
                ImGui::TreePop();
            }
        }
    }
#endif
}

void Scene::Cleanup()
{
    LOG("Cleaning up scene...");

    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();
    static MeshManager& mm = WindowsEngine::GetModule<MeshManager>();

    shouldStopLoading = true;
    if (loadingThread.joinable())
        loadingThread.join();
    shouldStopLoading = false;

    data.Clear();

    WindowsEngine::GetModule<CameraManager>().Cleanup();

    // TODO: refactor this into assetManager.Cleanup();
    mm.SoftCleanup();
    tm.SoftCleanup();

    LOG("Scene cleaned up");
}

void Scene::Reset()
{
    LoadScene(data.sourceFilename);
}

void Scene::ReloadShaders() const
{
    for (const auto& grassPatch : data.grassPatches)
    {
        grassPatch->ReloadShaders();
    }
}

SceneDrawContext::SceneDrawContext(RendererModule* renderer, D3D11Device* device, const DirectX::BoundingFrustum& cam)
    : DrawContext{ renderer, device }
    , cameraFrustum{cam}
{}

bool SceneDrawContext::ShouldBeCulled(const DirectX::BoundingBox& bb)
{
    const bool cull = !cameraFrustum.Intersects(bb);
#if _DEBUG
    if (renderer->drawBoundingBoxes)
        DrawBoundingBox(bb, cull);
#endif
    return cull;
}

bool SceneDrawContext::ShouldBeCulled(const DirectX::BoundingOrientedBox& obb)
{
    const bool cull = !cameraFrustum.Intersects(obb);
#ifdef _DEBUG
    if (renderer->drawBoundingBoxes)
        DrawBoundingBox(obb, cull);
#endif
    return cull;
}

bool SceneDrawContext::ShouldBeCulled(const DirectX::BoundingSphere& bs)
{
    return !cameraFrustum.Intersects(bs);
}

}
