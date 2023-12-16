#pragma once
#include "Texture2D.h"
#include "Core/SceneParser.h"
#include "Lights/DirectionalLight.h"

namespace Snail
{
class D3D11Device;
class ComputeShader;

class VolumetricLighting
{
public:
    VolumetricLighting(D3D11Device* device);
    ~VolumetricLighting();
    void Render();
    void Resize(long width, long height);
    void Clear();
    void ReloadShaders();

    [[nodiscard]] bool IsActive() const noexcept;
    [[nodiscard]] Texture2D* GetVolumetricAccumulationBuffer() const noexcept;

private:
    struct HalfResAccumulationData
    {
        Matrix invProjectionMatrix;
        Matrix invViewMatrix;
        DX_ALIGN uint32_t nbDirectional;
        struct DX_ALIGN RayIdx {
            int val1;
            int val2;
            int val3;
            int val4;
        } randomRayIndices[16];
    } halfResAccumulationData{};
    D3D11Buffer halfResAccumulationDataBuffer;

    std::array<int, 64> randomRayIndices;

    D3D11Device* renderDevice = nullptr;
    bool isActive = true;
    bool doBlur = true;

    std::unique_ptr<Texture2D> halfResAccumulationTexture;
    ID3D11UnorderedAccessView* halfResAccumulationUAV = nullptr;

    std::unique_ptr<Texture2D> halfResDepthTexture;
    ID3D11UnorderedAccessView* halfResDepthUAV = nullptr;

    std::unique_ptr<Texture2D> finalAccumulationTexture;
    ID3D11UnorderedAccessView* finalAccumulationUAV = nullptr;

    ID3D11SamplerState* comparisonFilteringSampler = nullptr;
    ID3D11SamplerState* pointFilteringSampler = nullptr;
    ID3D11SamplerState* bilinearFilteringSampler = nullptr;

    std::unique_ptr<ComputeShader> halfResAccumulationPass;
    std::unique_ptr<ComputeShader> halfResDepthPass;
    std::unique_ptr<ComputeShader> bilateralGaussianBlurPass;
    std::unique_ptr<ComputeShader> upsamplePass;

    void InitTextures(long width, long height);
    void RenderHalfResAccumulationPass();
    void RenderHalfResDepthPass();
    void RenderBilateralGaussianBlurPass();
    void RenderUpsamplePass();

public:
    void RenderImGui();
#ifdef _IMGUI_
    bool debugVolumetricShadows = false;
#endif
};

}
