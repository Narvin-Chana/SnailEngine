#include "stdafx.h"
#include "RendererModule.h"

#include "Rendering/D3D11Device.h"
#include "Rendering/MeshVertex.h"
#include "WindowsEngine.h"
#include "Mesh/BillboardMesh.h"
#include "Mesh/Mesh.h"
#include "Mesh/DecalMesh.h"
#include "Rendering/InputAssembler.h"
#include "Rendering/Effects/ScreenShakeEffect.h"
#include "Rendering/Effects/PostProcessing/BlurEffect.h"
#include "Rendering/Effects/PostProcessing/SSAOEffect.h"
#include "Rendering/Effects/PostProcessing/VignetteEffect.h"
#include "Rendering/Effects/PostProcessing/ChromaticAberrationEffect.h"
#include "Rendering/Shadows/DirectionalShadowMap.h"

namespace Snail
{
void RendererModule::Init()
{
    std::lock_guard lock{DeviceMutex};

    static WindowsEngine& engine = WindowsEngine::GetInstance();
    hMainWnd = engine.GetHwnd();

#ifdef _DEBUG
    debugLinesMVPBuffer = D3D11Buffer::CreateConstantBuffer<Matrix>();
#endif

#ifdef _IMGUI_
    gBufferIndexBuffer = D3D11Buffer::CreateConstantBuffer<int>();
#endif

    device = engine.GetRenderDevice();

    device->InitGBuffer();
    LOG("Created GBuffers");

    dirshadowMap = std::make_unique<DirectionalShadowMap>(device);

    volumetricLighting = std::make_unique<VolumetricLighting>(device);

    lightingPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/LightingPass.fx",
        DEFAULT_ELEMENT_LAYOUT,
        DEFAULT_ELEMENT_COUNT,
        {PCF_DEFINE, VOLUMETRIC_LIGHTS_DEFINE}));
#ifdef _IMGUI_
    if (activateFXAA)
        finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/FXAAPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
    else
        finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/ConvertPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
#else
    finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/FXAAPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
#endif
    LOG("Created core shaders");

    vignetteEffect = std::make_unique<VignetteEffect>();
    vignetteEffect->SetActive(true);
    ssaoEffect = std::make_unique<SSAOEffect>(device);
    ssaoEffect->SetActive(true);
    blurEffect = std::make_unique<BlurEffect>();
    chromaticAberrationEffect = std::make_unique<ChromaticAberrationEffect>();
    LOG("Created post-process shaders");

#ifdef _DEBUG
    debugLineShader.reset(new EffectsShader(L"SnailEngine/Shaders/DebugLines.fx", DebugLine::layout, DebugLine::elementCount));
#endif

#ifdef _RENDERDOC_
        if (const HMODULE mod = GetModuleHandle(L"renderdoc.dll"))
        {
            const pRENDERDOC_GetAPI renderdocGetApi = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
            const int ret = renderdocGetApi(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&rdoc_api));

            if (!ret)
                MessageBox(hMainWnd, L"Render doc injection failed", L"oops", MB_OK);

            UNREFERENCED_PARAMETER(ret);
        }
#endif

#ifdef _IMGUI_
    imGuiEffectsShader.reset(new EffectsShader(L"SnailEngine/Shaders/Default.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(hMainWnd);

    ImGui_ImplDX11_Init(device->GetD3DDevice(), device->GetImmediateContext());

    ImGui::StyleColorsDark();
#endif
}

void RendererModule::DrawPostEffects() const
{
    vignetteEffect->RenderEffect(device);
    chromaticAberrationEffect->RenderEffect(device);
}

void RendererModule::DrawFinalPassFXAA()
{
    // Copy final UAV to RTV for showing to back buffer
#ifdef _IMGUI_
    if (activateFXAA)
    {
#endif
        // At the same time apply FXAA
        auto res = device->GetResolutionSize();
        fxaaBufferData.inverseScreenSize = Vector2{ 1.0f / res.x, 1.0f / res.y };
        fxaaBuffer.UpdateData(fxaaBufferData);
        finalFXAAPassShader->SetConstantBuffer("FXAAParameters", fxaaBuffer.GetBuffer());
#ifdef _IMGUI_
    }
#endif
    finalFXAAPassShader->BindShaderResourceViewAndSampler("UAV", device->GetPostProcessSRV(), device->GetPostProcessSamplerState());
    finalFXAAPassShader->Bind();

    device->GetImmediateContext()->Draw(6, 0);
}

void RendererModule::DrawUI()
{
    const auto& camera = WindowsEngine::GetCamera();
    camera->DrawOverlays();
}

void RendererModule::Update(const float dt)
{
    ssaoEffect->Update(dt);
    blurEffect->Update(dt);
    screenShakeEffect.Update(dt);
    vignetteEffect->Update(dt);
    chromaticAberrationEffect->Update(dt);
}

void RendererModule::BeginRenderScene()
{
#ifdef _RENDERDOC_
        if (rdoc_api)
            rdoc_api->StartFrameCapture(nullptr, hMainWnd);
#endif

    device->Clear();
    if (volumetricLighting->IsActive())
    {
        volumetricLighting->Clear();
    }

    if (screenShakeEffect.IsActive())
    {
        screenShakeEffect.RenderEffectBegin();
    }
}

#ifdef _DEBUG
void RendererModule::DrawLines()
{
    if (debugLines.empty())
    {
        return;
    }

    const Matrix vpMatrix = WindowsEngine::GetCamera()->GetViewProjectionMatrix();

    debugLinesMVPBuffer.UpdateData(vpMatrix);
    debugLineShader->SetConstantBuffer("TransformMatrixes", debugLinesMVPBuffer.GetBuffer());

    // Add lines data to buffers
    debugLinesVertexBuffer.UpdateData<std::vector<DebugLine>>(debugLines);

    InputAssembler::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    InputAssembler::SetVertexBuffer(debugLinesVertexBuffer, sizeof(DebugLine), 0);

    debugLineShader->Bind();

    device->Draw(static_cast<unsigned int>(debugLines.size()));

    debugLines.clear();
}

#endif

void RendererModule::DrawMeshes(std::function<bool(BaseMesh*)> filter) const
{
    static auto& engine = WindowsEngine::GetInstance();
    static auto& mm = engine.GetModule<MeshManager>();

    const auto& buff = engine.GetCamera()->GetTransformMatrixesBuffer();

    for (BaseMesh* mesh : mm.GetAllAssets() | std::views::filter(filter))
    {
        mesh->Draw(&buff);
    }
}

void RendererModule::DrawMeshesGeometry() const
{
    static auto& engine = WindowsEngine::GetInstance();
    static auto& mm = engine.GetModule<MeshManager>();

    for (BaseMesh* mesh : mm.GetAllAssets())
    {
        mesh->DrawGeometry();
    }
}

void RendererModule::DrawLighting(Scene* scene)
{
#ifdef _IMGUI_
    if (currentBufferToShow != -1)
    {
        gBufferIndexBuffer.UpdateData(currentBufferToShow);
        imGuiEffectsShader->SetConstantBuffer("IndexGBuffer", gBufferIndexBuffer.GetBuffer());
        imGuiEffectsShader->BindTexture("GBuffer", device->GetGBufferTexture2DArray());
        imGuiEffectsShader->Bind();

        device->GetImmediateContext()->Draw(6, 0);
        return;
    }
#endif
    const auto& dirLights = scene->GetDirectionalLightsBuffer();
    const auto& spotLights = scene->GetSpotLightsBuffer();
    const auto& ptLights = scene->GetPointLightsBuffer();
    const auto& psBuff = scene->GetSceneInfoBuffer();
    lightingPassShader->SetConstantBuffer("SceneInfo", psBuff.GetBuffer());
    lightingPassShader->SetConstantBuffer("SpotLights", spotLights.GetBuffer());
    lightingPassShader->SetConstantBuffer("PointLights", ptLights.GetBuffer());
    lightingPassShader->SetConstantBuffer("DirectionalLights", dirLights.GetBuffer());
    lightingPassShader->SetConstantBuffer("DirectionalLightShadows", dirshadowMap->GetViewProjBuffer(scene->GetDirectionalLights()).GetBuffer());

    postProcessBufferData.SSAOEnabled = ssaoEffect->IsActive();
    postProcessBuffer.UpdateData(postProcessBufferData);
    lightingPassShader->SetConstantBuffer("PostProcess", postProcessBuffer.GetBuffer());

    lightingPassShader->BindTexture("GBuffer", device->GetGBufferTexture2DArray());
    lightingPassShader->BindShaderResourceView("DepthTexture", device->GetDepthShaderResourceView());
    lightingPassShader->BindShaderResourceView("SSAOTexture", ssaoEffect->GetSSAOSRV());
    lightingPassShader->BindTexture("DirectionalShadowMap", dirshadowMap->GetDepthTexture());
    if (volumetricLighting->IsActive())
    {
        lightingPassShader->BindShaderResourceView("VolumetricAccumulationBuffer", volumetricLighting->GetVolumetricAccumulationBuffer()->GetShaderResourceView());
    }

    lightingPassShader->Bind();

    device->GetImmediateContext()->Draw(6, 0);

    // Unbind depth texture from lightingPassShader to be able to draw to it next pass without any warnings
    lightingPassShader->UnbindResource("DirectionalShadowMap");
    lightingPassShader->UnbindResource("DepthTexture");
    lightingPassShader->UnbindResource("SSAOTexture");
    lightingPassShader->UnbindResource("GBuffer");
    if (volumetricLighting->IsActive())
    {
        lightingPassShader->UnbindResource("VolumetricAccumulationBuffer");
    }
}

void RendererModule::EndRenderScene()
{
    // Reset camera to original position
    if (screenShakeEffect.IsActive())
    {
        screenShakeEffect.RenderEffectEnd();
    }

#ifdef _RENDERDOC_
    // Renderdoc api calls
    if (rdoc_api)
    {
        rdoc_api->EndFrameCapture(nullptr, hMainWnd);
        rdoc_api = nullptr;
    }
#endif

#ifdef _IMGUI_
    // ImGui api calls
    ImGui::End();
    // Assemble together draw data
    ImGui::Render();
    // Render draw data
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
}

RendererModule::RendererModule()
    : postProcessBuffer{D3D11Buffer::CreateConstantBuffer<PostProcessBufferData>()}
    , fxaaBuffer{D3D11Buffer::CreateConstantBuffer<FXAABufferData>()}
#ifdef _DEBUG
    , debugLinesVertexBuffer{D3D11_BIND_VERTEX_BUFFER}
#ifdef _IMGUI_
    , currentBufferToShow(-1)
#endif
#endif
{}

RendererModule::~RendererModule()
{
#ifdef _IMGUI_
    // Only cleanup when its been init
    if (ImGui::GetCurrentContext())
    {
        // Releases COM references that ImGui was given on setup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
#endif
}

void RendererModule::Render(Scene* scene)
{
    static WindowsEngine& engine = WindowsEngine::GetInstance();

    const Camera* frustumCamera = WindowsEngine::GetModule<CameraManager>().GetCamera(0);
    if (!frustumCamera)
    {
        LOG(Logger::ERROR, "Couldn't draw because no camera was set");
        return;
    }

    DirectX::BoundingFrustum frustum;
    if (frustumCamera->IsPerspectiveCamera())
        frustum = GetFrustumFromCamera(frustumCamera);

    SceneDrawContext ctx{&engine.GetModule<RendererModule>(), engine.GetRenderDevice(), frustum};

    BeginRenderScene();

    RenderImGui();
    scene->RenderImGui();

    dirshadowMap->Render(scene->GetDirectionalLights());

    // Draw only scene geometry
    device->PrepareDeferredDraw();
    scene->Draw(ctx);
    device->SetFrontFaceCulling();
    DrawMeshes([](BaseMesh* m)
    {
        return dynamic_cast<DecalMesh*>(m) == nullptr && dynamic_cast<BillboardMesh*>(m) == nullptr;
    });

#ifdef _DEBUG
    DrawLines();
#endif

    device->SetAlpha(true);
    DrawMeshes([](BaseMesh* m)
    {
        return dynamic_cast<BillboardMesh*>(m) != nullptr;
    });

    // Draw all decals last (requires unbinding depth)
    device->PrepareDecalDraw();
    scene->DrawDecals(ctx);
    DrawMeshes([](BaseMesh* m)
    {
        return dynamic_cast<DecalMesh*>(m) != nullptr;
    });

    device->PrepareVolumetricDraw();
    volumetricLighting->Render();

    // Perform deferred lighting draw
    device->PrepareLightingDraw();
    ssaoEffect->RenderEffect(device);
    DrawLighting(scene);

    // Draw skybox after deferred to avoid overdraw
    device->PrepareSkyboxDraw();
    scene->DrawSkybox(ctx);

    // Perform all post processing with compute shaders taking in (and drawing to) a singular UAV
    device->PreparePostProcessDraw();
    DrawPostEffects();

    // Final draw to main back-buffer RTV and perform FXAA at the same time
    device->PrepareDrawToFinalRTV();
    DrawFinalPassFXAA();

    // Draw UI above everything
    device->SetNoCulling();
    device->SetAlpha(true);
    DrawUI();

    EndRenderScene();
}
void RendererModule::Resize(const long width, const long height) const
{
    ssaoEffect->Resize(width, height);
    volumetricLighting->Resize(width, height);
}

#ifdef _DEBUG
void RendererModule::DrawLine(const DebugLine& startLine, const DebugLine& endLine)
{
    debugLines.push_back(startLine);
    debugLines.push_back(endLine);
}

void RendererModule::DrawFrustum(const DirectX::BoundingFrustum& frustum, const Color& frustumColor)
{
    DirectX::XMFLOAT3 corners[8];
    frustum.GetCorners(corners);

    DrawLine({corners[0], frustumColor}, {corners[1], frustumColor});
    DrawLine({corners[1], frustumColor}, {corners[2], frustumColor});
    DrawLine({corners[2], frustumColor}, {corners[3], frustumColor});
    DrawLine({corners[3], frustumColor}, {corners[0], frustumColor});

    DrawLine({corners[4], frustumColor}, {corners[5], frustumColor});
    DrawLine({corners[5], frustumColor}, {corners[6], frustumColor});
    DrawLine({corners[6], frustumColor}, {corners[7], frustumColor});
    DrawLine({corners[7], frustumColor}, {corners[4], frustumColor});

    DrawLine({corners[0], frustumColor}, {corners[4], frustumColor});
    DrawLine({corners[1], frustumColor}, {corners[5], frustumColor});
    DrawLine({corners[2], frustumColor}, {corners[6], frustumColor});
    DrawLine({corners[3], frustumColor}, {corners[7], frustumColor});
}

void RendererModule::DrawBoundingBox(const DirectX::BoundingBox& boundingBox, const Color& boundingColor)
{
    std::array<Vector3, 8> corners;
    boundingBox.GetCorners(corners.data());

    DrawBoundingBox(corners, boundingColor);
}

void RendererModule::DrawBoundingBox(const DirectX::BoundingOrientedBox& boundingBox, const Color& boundingColor)
{
    std::array<Vector3, 8> corners;
    boundingBox.GetCorners(corners.data());

    DrawBoundingBox(corners, boundingColor);
}

void RendererModule::DrawBoundingBox(const std::array<Vector3, 8>& boundingBox, const Color& boundingColor)
{
    DrawLine({boundingBox[0], boundingColor}, {boundingBox[1], boundingColor});
    DrawLine({boundingBox[1], boundingColor}, {boundingBox[2], boundingColor});
    DrawLine({boundingBox[2], boundingColor}, {boundingBox[3], boundingColor});
    DrawLine({boundingBox[3], boundingColor}, {boundingBox[0], boundingColor});

    DrawLine({boundingBox[4], boundingColor}, {boundingBox[5], boundingColor});
    DrawLine({boundingBox[5], boundingColor}, {boundingBox[6], boundingColor});
    DrawLine({boundingBox[6], boundingColor}, {boundingBox[7], boundingColor});
    DrawLine({boundingBox[7], boundingColor}, {boundingBox[4], boundingColor});

    DrawLine({boundingBox[0], boundingColor}, {boundingBox[4], boundingColor});
    DrawLine({boundingBox[1], boundingColor}, {boundingBox[5], boundingColor});
    DrawLine({boundingBox[2], boundingColor}, {boundingBox[6], boundingColor});
    DrawLine({boundingBox[3], boundingColor}, {boundingBox[7], boundingColor});
}
#else
    void RendererModule::DrawBoundingBox(const DirectX::BoundingOrientedBox&, const Color&) {}
    void RendererModule::DrawFrustum(const DirectX::BoundingFrustum&, const Color&) {}
    void RendererModule::DrawBoundingBox(const DirectX::BoundingBox&, const Color&) {}
    void RendererModule::DrawLine(const DebugLine&, const DebugLine&) {}
    void RendererModule::DrawBoundingBox(const std::array<Vector3, 8>&, const Color&) {}
#endif

void RendererModule::RenderImGui()
{
#ifdef _IMGUI_
    static WindowsEngine& engine = WindowsEngine::GetInstance();

    static MeshManager& mm = engine.GetModule<MeshManager>();
    static TextureManager& tm = engine.GetModule<TextureManager>();

    // Only render ImGui window after everything else is done (will call ImGui::End() on frame render end)
    // Start frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Game");

    static GameManager& gameManager = engine.GetModule<GameManager>();
    gameManager.RenderImGui();

    ImGui::End();

    // Create window
    ImGui::Begin("Snail");

#ifdef _DEBUG
    if (ImGui::Button("Report D3D11 Live Objects"))
    {
        if (device->debugDev)
            device->debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
#endif

    ImGui::Checkbox("VSync", &device->VSyncEnabled);

    static float elapsedTime = 0;
    auto dt = engine.GetDeltaTime();
    static auto FPS = 1 / dt;
    if (elapsedTime > 0.5)
    {
        FPS = 1 / dt;
        elapsedTime = 0;
    }
    ImGui::Text(("FPS: " + std::to_string(FPS)).c_str());
    elapsedTime += dt;

    ImGui::Separator();

    ImGui::Text("Debug Rendering:");

    if (ImGui::Button("Recompile Shaders"))
    {
        imGuiEffectsShader->ReloadShader();

        const std::vector<BaseMesh*> meshes = mm.GetAllAssets();
        for (auto* mesh : meshes)
        {
            mesh->ReloadShader();
        }
        engine.GetScene()->ReloadShaders();
        lightingPassShader->ReloadShader();

        if (activateFXAA)
            finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/FXAAPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
        else
            finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/ConvertPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));

        vignetteEffect->ReloadShader();
        ssaoEffect->ReloadShader();
        blurEffect->ReloadShader();
        chromaticAberrationEffect->ReloadShader();
        volumetricLighting->ReloadShaders();
    }

    ImGui::Text("Choose rendering pass to show: ");
    static constexpr std::array possiblePasses = {"Default", "WorldPosition", "Normal", "Albedo", "Specular", "Emission", "Unlit"};
    static const char* currentItem = "Default";

    if (ImGui::BeginCombo("##render pass", currentItem))
    {
        for (int n = 0; n < possiblePasses.size(); n++)
        {
            const bool isSelected = (currentItem == possiblePasses[n]);
            if (ImGui::Selectable(possiblePasses[n], isSelected))
            {
                currentItem = possiblePasses[n];
                currentBufferToShow = n - 1;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::Checkbox("Draw Entity Transforms", &drawEntityTransform);
    ImGui::Checkbox("Draw Bounding Boxes", &drawBoundingBoxes);

    if (ImGui::Checkbox("FXAA Active", &activateFXAA))
    {
        if (activateFXAA)
            finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/FXAAPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
        else
            finalFXAAPassShader.reset(new EffectsShader(L"SnailEngine/Shaders/ConvertPass.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT));
    }

    bool hasPCF = lightingPassShader->HasDefine(PCF_DEFINE);
    if (ImGui::Checkbox("PCF Shadows", &hasPCF))
    {
        if (hasPCF)
            lightingPassShader->AddDefine(PCF_DEFINE);
        else
            lightingPassShader->RemoveDefine(PCF_DEFINE);
    }

    if (ImGui::Checkbox("Debug shadows", &debugShadows))
    {
        if (debugShadows)
            lightingPassShader->AddDefine(SHADOWS_DEBUG_DEFINE);
        else
            lightingPassShader->RemoveDefine(SHADOWS_DEBUG_DEFINE);
    }

    volumetricLighting->RenderImGui();

    ImGui::Separator();

    if (ImGui::CollapsingHeader("General Effects:"))
    {
        screenShakeEffect.RenderImGui();
    }

    if (ImGui::CollapsingHeader("Post-Processing Effects:"))
    {
        vignetteEffect->RenderImGui();
        ssaoEffect->RenderImGui();
        //blurEffect->RenderImGui();
        chromaticAberrationEffect->RenderImGui();
    }

    ImGui::Separator();

    engine.GetScene()->RenderGrassImGui();

    ImGui::Separator();

    dirshadowMap->RenderImGui();

    ImGui::Separator();

    // Improve this by using camera manager in engine.h and calling render imgui on it instead
    WindowsEngine::GetCamera()->RenderImGui();

    ImGui::Separator();

    mm.RenderImGui();

    ImGui::Separator();

    tm.RenderImGui();
#endif
}

}
