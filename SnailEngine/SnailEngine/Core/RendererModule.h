#pragma once
#include <memory>

#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/Effects/ScreenShakeEffect.h"
#include "Util/Util.h"
#include "Entities/GrassGenerator.h"
#include "Rendering/VolumetricLighting.h"

namespace Snail
{
class ChromaticAberrationEffect;
class VignetteEffect;
class EffectsShader;
class DirectionalShadowMap;
class Scene;
class SSAOEffect;
class BaseMesh;
class BlurEffect;

class RendererModule
{
public:
    struct DX_ALIGN DebugLine
    {
        inline static D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        inline static UINT elementCount = ARRAYSIZE(layout);

        Vector3 position;
        Color color;
    };
#ifdef _IMGUI_
    bool drawEntityTransform = false;
    bool debugShadows = false;
    inline static constexpr const char* SHADOWS_DEBUG_DEFINE = "DEBUG_SHADOWS";
    inline static constexpr const char* VOLUMETRIC_LIGHTING_DEBUG_DEFINE = "DEBUG_VOLUMETRIC";
    friend class VolumetricLighting;
#endif

private:
    struct PostProcessBufferData
    {
        int SSAOEnabled{};
    } postProcessBufferData{};

    D3D11Buffer postProcessBuffer;

    struct FXAABufferData
    {
        Vector2 inverseScreenSize;
    } fxaaBufferData{};

    D3D11Buffer fxaaBuffer;

    inline static constexpr const char* PCF_DEFINE = "PCF";
    inline static constexpr const char* VOLUMETRIC_LIGHTS_DEFINE = "VOLUMETRIC_LIGHTING";

    HWND hMainWnd = nullptr;
    D3D11Device* device = nullptr;
    std::unique_ptr<DirectionalShadowMap> dirshadowMap;
    std::unique_ptr<EffectsShader> lightingPassShader;
    std::unique_ptr<EffectsShader> finalFXAAPassShader;

    std::unique_ptr<VolumetricLighting> volumetricLighting;

#ifdef _DEBUG
    D3D11Buffer debugLinesVertexBuffer;
    D3D11Buffer debugLinesMVPBuffer;
#endif

    std::vector<DebugLine> debugLines;
    std::unique_ptr<EffectsShader> debugLineShader;

public:
    std::unique_ptr<VignetteEffect> vignetteEffect;
    std::unique_ptr<SSAOEffect> ssaoEffect;
    std::unique_ptr<BlurEffect> blurEffect;
    std::unique_ptr<ChromaticAberrationEffect> chromaticAberrationEffect;
    ScreenShakeEffect screenShakeEffect;

private:


#ifdef _RENDERDOC_
    HINSTANCE RenderDocDLL{};
    RENDERDOC_API_1_1_2* rdoc_api = nullptr;
#endif

    void BeginRenderScene();
    void RenderImGui();
    void DrawLighting(Scene* scene);
    void DrawPostEffects() const;
    void DrawLines();
    void DrawMeshes(std::function<bool(BaseMesh*)> filter = [](BaseMesh*) { return true; }) const;
    void DrawFinalPassFXAA();
    void EndRenderScene();

public:
    void DrawMeshesGeometry() const;
    RendererModule();
    ~RendererModule();
    void Init();
    void DrawUI();
    void Update(float dt);
    void Render(Scene* scene);
    void Resize(long width, long height) const;

    void DrawLine(const DebugLine& startLine, const DebugLine& endLine);
    void DrawFrustum(const DirectX::BoundingFrustum& frustum, const Color& frustumColor);
    void DrawBoundingBox(const DirectX::BoundingBox& boundingBox, const Color& boundingColor);
    void DrawBoundingBox(const DirectX::BoundingOrientedBox& boundingBox, const Color& boundingColor);
    void DrawBoundingBox(const std::array<Vector3, 8>& boundingBox, const Color& boundingColor);

    DirectionalShadowMap* GetDirectionalShadowMap() const noexcept { return dirshadowMap.get(); }
    VolumetricLighting* GetVolumetricLighting() const noexcept { return volumetricLighting.get(); }

#ifdef _IMGUI_
    std::unique_ptr<EffectsShader> imGuiEffectsShader;
    D3D11Buffer gBufferIndexBuffer;
    int currentBufferToShow = -1;
    bool drawBoundingBoxes = false;
    bool usePCF = true;
    bool activateFXAA = true;
#endif

};
}
