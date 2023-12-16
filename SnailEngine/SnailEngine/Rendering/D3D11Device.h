#pragma once
#include <memory>
#include <windef.h>

#include "Texture2D.h"
#include "Rendering/Device.h"

namespace Snail
{
class D3D11Device final : public Device
{
public:
    D3D11Device(DisplayMode cdsMode, HWND hWnd);
    ~D3D11Device() override;
    void InitGBuffer() override;
    void PresentSpecific() override;

    DirectX::XMINT2 GetFullscreenResolution() const;
    DirectX::XMINT2 GetWindowedResolution() const;

    void SetResolution(long, long) override;
    void SetDisplayMode(DisplayMode mode) override;

    void DrawIndexed(unsigned int vertexCount, unsigned int startVertex = 0) override;
    void DrawIndexedInstanced(
        int indexCountPerInstance,
        int instanceCount,
        int startIndexLocation = 0,
        int baseVertexLocation = 0,
        int startInstanceLocation = 0) override;
    void DrawIndexedInstanced(uint32_t indexBufferCount, int instancesCount, uint32_t indexBufferStartIndex);
    void Draw(unsigned int vertexCount) override;

    void PrepareDeferredDraw() override;
    void PrepareDecalDraw() override;
    void PrepareVolumetricDraw() override;
    void PrepareLightingDraw() override;
    void PrepareSkyboxDraw() override;
    void PreparePostProcessDraw() override;
    void PrepareDrawToFinalRTV();

    void SetFrontFaceCulling() const;
    void SetBackFaceCulling() const;
    void SetWireframe();
    void SetNoCulling() const;
    void SetAlpha(bool activateAlpha);
    void SetFill();

    void SetClearColor(const Color& color);
    void Clear();
    void ClearRenderTarget();

    void ResetRenderTarget();
    void ResetViewPort();

    ID3D11Device* GetD3DDevice() const noexcept { return device; }

    ID3D11DeviceContext* GetImmediateContext() const noexcept { return immediateContext; }

    IDXGISwapChain* GetSwapChain() const noexcept { return swapChain; }

    ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return renderTargetView; }

    ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return depthStencilView; }
    ID3D11ShaderResourceView* GetDepthShaderResourceView() const noexcept { return depthShaderResource; }

    Texture2D* GetGBufferTexture2DArray() const { return deferredPassTexture.get(); }

    ID3D11Texture2D* GetPostProcessTexture() const { return postProcessTexture; }
    ID3D11SamplerState* GetPostProcessSamplerState() const { return postProcessSamplerState; }
    ID3D11ShaderResourceView* GetPostProcessSRV() const { return postProcessSRV; }
    ID3D11UnorderedAccessView* GetPostProcessUAV() const { return postProcessUAV; }

#ifdef _DEBUG
    ID3D11Debug* debugDev{};
#endif
#ifdef _PRIVATE_DATA
    static void SetDebugName(ID3D11DeviceChild* object, const std::string& name);
#endif
private:
    static constexpr int NUM_DEFERRED_TEXTURES = 6;

    ID3D11Device* device{};
    ID3D11DeviceContext* immediateContext{};
    IDXGISwapChain* swapChain{};
    ID3D11RenderTargetView* renderTargetView{};
    ID3D11Texture2D* depthTexture{};
    ID3D11DepthStencilView* depthStencilView{};
    ID3D11ShaderResourceView* depthShaderResource{};
    ID3D11DepthStencilState* depthStencilState{};
    ID3D11RasterizerState* solidCullBackRs{};
    ID3D11RasterizerState* solidCullFrontRs{};
    ID3D11RasterizerState* wireframeRs{};
    ID3D11RasterizerState* noCullingRs{};
    ID3D11RasterizerState* prevRs{};

    ID3D11BlendState* alphaBlendEnable{};
    ID3D11BlendState* alphaBlendDisable{};

    ID3D11RenderTargetView* renderTargetViewDeferred[NUM_DEFERRED_TEXTURES]{};
    std::unique_ptr<Texture2D> deferredPassTexture{};

    ID3D11Texture2D* postProcessTexture{};
    ID3D11SamplerState* postProcessSamplerState{};
    ID3D11RenderTargetView* postProcessRTV{};
    ID3D11UnorderedAccessView* postProcessUAV{};
    ID3D11ShaderResourceView* postProcessSRV{};

    HWND hWnd;
    Color clearColor = { 0.5f, 0.7f, 0.5f };

    void InitDepthBuffer();
    void InitRasterizerStates();
    void InitBlendStates();
    void InitPostProcessUAV();
    void SetViewPort(const DirectX::XMINT2& size) const;
};
}
