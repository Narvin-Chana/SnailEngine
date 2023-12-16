#pragma once

#include <memory>

#include "Rendering/Texture2D.h"
#include "Rendering/Buffers/StructuredBuffer.h"
#include "Rendering/Shaders/ComputeShader.h"
#include "Rendering/Shaders/EffectsShader.h"
#include "Util/Util.h"

namespace Snail
{
class DrawContext;

class GrassGenerator
{
    constexpr static int VerticesPerGrassBladeCount = 9;
    // This is hard coded as compute shaders should avoid using more than 8x8
    constexpr static int BladeCountPerRegion = 64;
    constexpr static std::array<uint32_t, 2> DispatchThreadCount = {8, 8};

    struct GrassVertex
    {
        Vector3 position;
        Vector2 uv;
        static inline D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        static inline UINT elementCount = ARRAYSIZE(layout);
    };

    struct GrassInstancedData
    {
        Vector3 worldPosition;
        Vector3 baseColor;
        Vector3 tipColor;
        float randomAngle;
        float randomShade;
        float randomHeight;
        float randomLean;
    };

    struct GrassComputeParams
    {
        Matrix grassDensityScaleMatrix;
        DX_ALIGN Vector3 worldPosition;
        DX_ALIGN std::array<uint32_t, 2> groupCount;
    };

    struct GrassEffectsParams
    {
        Matrix matWorldViewProj;
        Matrix matWorld;
        Matrix matView;
        Matrix matViewInv;
        DX_ALIGN float time;
    };

    D3D11Buffer grassComputeConstantBuffer;
    D3D11Buffer grassEffectsConstantBuffer;

    D3D11Buffer indexBuffer;
    D3D11Buffer vertexBuffer;

    StructuredBuffer instancedDataBuffer;

    std::unique_ptr<ComputeShader> generateGrassInstanceDataComputeShader;
    std::unique_ptr<EffectsShader> grassEffectsShader;
    std::unique_ptr<EffectsShader> grassShadowsEffectsShader;

    Texture2D* sampleTexture = nullptr;

    std::array<uint32_t, 2> regionCount = {8, 8};

    Transform grassPatchPosition;
    float grassDensityScale;

    float totalTime = 0;

#ifdef _IMGUI_
    bool liveRegenerateGrassInstanceData = false;
#endif

    void DoDrawCall() const;

public:
    GrassGenerator(float grassDensityScale, std::array<uint32_t, 2> regionCount, Transform grassPatchPosition, Texture2D* sampleTexture);
    ~GrassGenerator();
    void Update(float dt);
    void UpdateGrassBuffer();
    void Draw(DrawContext& ctx);
    void DrawShadows(D3D11Buffer& lightMatrixBuffer);

    void GenerateInstanceData();

    DirectX::BoundingOrientedBox GetBoundingBox() const;

    void ReloadShaders();
    void RenderImGui();
};

}
