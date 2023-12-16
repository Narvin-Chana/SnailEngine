#include "stdafx.h"
#include "Rendering/D3D11Device.h"

#include "InputAssembler.h"
#include "Texture2D.h"
#include "Rendering/DeviceInfo.h"
#include "Core/WindowsResource/Resource.h"
#include "Util/Util.h"

namespace Snail
{

D3D11Device::D3D11Device(const DisplayMode cdsMode, const HWND hWnd)
    : hWnd(hWnd)
{
    int width = 0;
    int height = 0;
    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    constexpr D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,};

    constexpr UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    switch (cdsMode)
    {
    case DisplayMode::WINDOWED:
        swapChainDesc.Windowed = TRUE;
        resolutionSize = GetWindowedResolution();
        break;
    case DisplayMode::FULL_SCREEN:
        swapChainDesc.Windowed = FALSE;
        resolutionSize = GetFullscreenResolution();
        break;
    }

    width = resolutionSize.x;
    height = resolutionSize.y;

    DXGI_MODE_DESC desc;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Height = height;
    desc.Width = width;
    desc.RefreshRate.Numerator = 60;
    desc.RefreshRate.Denominator = 1;
    desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DeviceInfo devInfo(desc);
    devInfo.GetDesc(desc);

    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = desc.Format;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = desc.RefreshRate.Numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = desc.RefreshRate.Denominator;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    DX_CALL(
        D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &immediateContext ),
        DXE_ERREURCREATIONDEVICE);

    ID3D11Texture2D* backBuffer;
    DX_CALL(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBuffer)), DXE_ERREUROBTENTIONBUFFER);

#ifdef _DEBUG
    DX_CALL(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDev)), "Error getting debug device");
#endif

    DX_CALL(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView), DXE_ERREURCREATIONRENDERTARGET);

    InitDepthBuffer();

    InitPostProcessUAV();

    immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &renderTargetView, depthStencilView, 1, 1, &postProcessUAV, nullptr);
    immediateContext->OMSetDepthStencilState(depthStencilState, 0);

    DX_RELEASE(backBuffer);

    SetViewPort(resolutionSize);
    InitRasterizerStates();
    InitBlendStates();

    SetAlpha(false);
}

D3D11Device::~D3D11Device()
{
    swapChain->SetFullscreenState(FALSE, nullptr);
    if (immediateContext) { immediateContext->ClearState(); }

    DX_RELEASE(solidCullBackRs);
    DX_RELEASE(solidCullFrontRs);
    DX_RELEASE(wireframeRs);
    DX_RELEASE(noCullingRs);

    DX_RELEASE(alphaBlendEnable);
    DX_RELEASE(alphaBlendDisable);

    DX_RELEASE(depthStencilState);
    DX_RELEASE(depthStencilView);
    DX_RELEASE(depthShaderResource);
    DX_RELEASE(depthTexture);
    DX_RELEASE(renderTargetView);
    DX_RELEASE(postProcessTexture);
    DX_RELEASE(postProcessUAV);
    DX_RELEASE(postProcessSRV);
    DX_RELEASE(postProcessRTV);
    for (int i = 0; i < NUM_DEFERRED_TEXTURES; ++i)
        DX_RELEASE(renderTargetViewDeferred[i]);

    DX_RELEASE(immediateContext);
    DX_RELEASE(swapChain);

#ifdef _DEBUG
    debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
    DX_RELEASE(debugDev);
#endif

    DX_RELEASE(device);
}

void D3D11Device::InitGBuffer()
{
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = resolutionSize.x;
    textureDesc.Height = resolutionSize.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 7;
    textureDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    deferredPassTexture.reset(new Texture2D(textureDesc, DXGI_FORMAT_R11G11B10_FLOAT));

    // Generate the render target view
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    renderTargetViewDesc.Texture2DArray.MipSlice = 0;
    renderTargetViewDesc.Texture2DArray.ArraySize = 1;

    for (int i = 0; i < NUM_DEFERRED_TEXTURES; ++i)
    {
        renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;

        DX_CALL(device->CreateRenderTargetView( deferredPassTexture.get()->GetRawTexture(), &renderTargetViewDesc, &renderTargetViewDeferred[i] ),
            DXE_ERREURCREATIONRENDERTARGET);
    }
}

void D3D11Device::PresentSpecific()
{
    // 1 means VSYNC on
    DX_CALL(swapChain->Present(VSyncEnabled, 0), "Error presenting swapchain.");
}

DirectX::XMINT2 D3D11Device::GetFullscreenResolution() const
{
    /*auto width = GetSystemMetrics(SM_CXSCREEN);
    auto height = GetSystemMetrics(SM_CYSCREEN);*/
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    return {rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top};
}

DirectX::XMINT2 D3D11Device::GetWindowedResolution() const
{
    RECT rcWindow;
    // Use getClientRect instead of getWindowRect to avoid offset problems
    // See: https://discourse.dearimgui.org/t/dx11-imgui-slightly-distorted-and-mouse-position-offset/60/8
    GetClientRect(hWnd, &rcWindow);
    return {rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top};
}

void D3D11Device::SetResolution(long width, long height)
{
    resolutionSize = {width, height};

    DX_RELEASE(depthStencilView);
    DX_RELEASE(depthTexture);
    DX_RELEASE(renderTargetView);
    DX_RELEASE(postProcessUAV);
    DX_RELEASE(postProcessSRV);
    for (int i = 0; i < NUM_DEFERRED_TEXTURES; ++i) { DX_RELEASE(renderTargetViewDeferred[i]); }

    DX_CALL(swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), 0);

    ID3D11Texture2D* backBuffer;
    DX_CALL(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBuffer)), DXE_ERREUROBTENTIONBUFFER);
    DX_CALL(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView), DXE_ERREURCREATIONRENDERTARGET);
    backBuffer->Release();

    InitDepthBuffer();

    InitPostProcessUAV();

    immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &renderTargetView, depthStencilView, 0, 0, nullptr, nullptr);
    immediateContext->OMSetDepthStencilState(depthStencilState, 0);

    InitGBuffer();

    SetViewPort(resolutionSize);
}

void D3D11Device::SetDisplayMode(const DisplayMode mode)
{
    if (mode == DisplayMode::FULL_SCREEN)
    {
        swapChain->SetFullscreenState(true, nullptr);
        const auto size = GetFullscreenResolution();
        SetResolution(size.x, size.y);
    }
    else
    {
        swapChain->SetFullscreenState(false, nullptr);
        const auto size = GetWindowedResolution();
        SetResolution(size.x, size.y);
    }
}

void D3D11Device::DrawIndexedInstanced(
    const int indexCountPerInstance,
    const int instanceCount,
    const int startIndexLocation,
    const int baseVertexLocation,
    const int startInstanceLocation)
{
    immediateContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void D3D11Device::DrawIndexed(const unsigned int vertexCount, const unsigned int startVertex)
{
    immediateContext->DrawIndexed(vertexCount, startVertex, 0);
}

void D3D11Device::Draw(const unsigned vertexCount)
{
    immediateContext->Draw(vertexCount, 0);
}

void D3D11Device::PrepareDeferredDraw()
{
    SetBackFaceCulling();
    ClearRenderTarget();
    SetAlpha(false);
    immediateContext->OMSetRenderTargets(NUM_DEFERRED_TEXTURES, renderTargetViewDeferred, depthStencilView);
}

void D3D11Device::PrepareDecalDraw()
{
    SetBackFaceCulling();
    ClearRenderTarget();
    SetAlpha(false);
    InputAssembler::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->OMSetRenderTargets(NUM_DEFERRED_TEXTURES, renderTargetViewDeferred, nullptr);
}

void D3D11Device::PrepareVolumetricDraw()
{
    ClearRenderTarget();
}

void D3D11Device::PrepareLightingDraw()
{
    SetBackFaceCulling();
    ClearRenderTarget();
    SetAlpha(false);
    InputAssembler::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->OMSetRenderTargets(1, &postProcessRTV, nullptr);
}

void D3D11Device::PrepareSkyboxDraw()
{
    SetFrontFaceCulling();
    SetAlpha(false);
    InputAssembler::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->OMSetRenderTargets(1, &postProcessRTV, depthStencilView);
}

void D3D11Device::PreparePostProcessDraw()
{
    // This method only unbinds the UAV to allow compute shaders to bind it later on.
    SetBackFaceCulling();
    ClearRenderTarget();
    SetAlpha(false);
}

void D3D11Device::PrepareDrawToFinalRTV()
{
    SetBackFaceCulling();
    ClearRenderTarget();
    SetAlpha(false);
    immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
}

void D3D11Device::SetFrontFaceCulling() const { immediateContext->RSSetState(solidCullFrontRs); }

void D3D11Device::SetBackFaceCulling() const { immediateContext->RSSetState(solidCullBackRs); }

void D3D11Device::SetWireframe()
{
    immediateContext->RSGetState(&prevRs);
    immediateContext->RSSetState(wireframeRs);
}

void D3D11Device::SetNoCulling() const { immediateContext->RSSetState(noCullingRs); }

void D3D11Device::SetAlpha(const bool activateAlpha)
{
    immediateContext->OMSetBlendState(activateAlpha ? alphaBlendEnable : alphaBlendDisable, nullptr, 0xffffffff);
}

void D3D11Device::SetFill()
{
    immediateContext->RSSetState(prevRs);
    prevRs = nullptr;
}

void D3D11Device::SetClearColor(const Color& color) { clearColor = color; }

void D3D11Device::Clear()
{
    const float color[4] = {clearColor.x, clearColor.y, clearColor.z, 1.0f};
    immediateContext->ClearRenderTargetView(renderTargetView, color);
    immediateContext->ClearUnorderedAccessViewFloat(postProcessUAV, color);

    constexpr float black[4]{};
    for (int i = 0; i < NUM_DEFERRED_TEXTURES; ++i)
    {
        // albedo should be cleared with clear color
        immediateContext->ClearRenderTargetView(renderTargetViewDeferred[i], i == 2 ? color : black);
    }

    immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 0.0, 0);
    SetBackFaceCulling();
}

void D3D11Device::ClearRenderTarget()
{
    // Have to pass in nullpointing pointer in order to unbind rtv & uav and use it as SRV
    ID3D11RenderTargetView* nullRTV = nullptr;
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &nullRTV, nullptr, 1, 1, &nullUAV, nullptr);
}

void D3D11Device::ResetRenderTarget()
{
    ClearRenderTarget();
    immediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void D3D11Device::ResetViewPort()
{
    BOOL fullscreen;
    swapChain->GetFullscreenState(&fullscreen, nullptr);

    if (fullscreen) { SetViewPort(GetFullscreenResolution()); }
    else { SetViewPort(GetWindowedResolution()); }
}

void D3D11Device::DrawIndexedInstanced(const uint32_t indexBufferCount, const int instancesCount, const uint32_t indexBufferStartIndex)
{
    immediateContext->DrawIndexedInstanced(indexBufferCount, instancesCount, indexBufferStartIndex, 0, 0);
}

#ifdef _PRIVATE_DATA
void D3D11Device::SetDebugName(ID3D11DeviceChild* object, const std::string& name)
{
    std::lock_guard lock{ DeviceMutex };
    object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
}
#endif

void D3D11Device::InitDepthBuffer()
{
    D3D11_TEXTURE2D_DESC depthTextureDesc;
    ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));

    const auto& [resolutionWidth, resolutionHeight] = resolutionSize;
    depthTextureDesc.Width = resolutionWidth;
    depthTextureDesc.Height = resolutionHeight;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthTextureDesc.CPUAccessFlags = 0;
    depthTextureDesc.MiscFlags = 0;
    DX_CALL(device->CreateTexture2D(&depthTextureDesc, nullptr, &depthTexture), DXE_ERREURCREATIONTEXTURE);

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    DX_CALL(device->CreateDepthStencilState(&dsDesc, &depthStencilState), DXE_ERROR_CREATE_SAMPLER_STATE);

    // Cr�ation de la vue du tampon de profondeur (ou de stencil)
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
    ZeroMemory(&descDSView, sizeof(descDSView));
    descDSView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSView.Texture2D.MipSlice = 0;
    DX_CALL(device->CreateDepthStencilView(depthTexture, &descDSView, &depthStencilView), DXE_ERREURCREATIONDEPTHSTENCILTARGET);

    D3D11_SHADER_RESOURCE_VIEW_DESC descDSRV;
    ZeroMemory(&descDSRV, sizeof(descDSRV));
    descDSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    descDSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    descDSRV.Texture2D.MipLevels = 1;
    descDSRV.Texture2D.MostDetailedMip = 0;
    DX_CALL(device->CreateShaderResourceView(depthTexture, &descDSRV, &depthShaderResource), DXE_ERREURCREATIONDEPTHSTENCILTARGET);
}

void D3D11Device::InitRasterizerStates()
{
    D3D11_RASTERIZER_DESC rsBackDesc;
    ZeroMemory(&rsBackDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsBackDesc.FillMode = D3D11_FILL_SOLID;
    rsBackDesc.CullMode = D3D11_CULL_BACK;
    rsBackDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&rsBackDesc, &solidCullBackRs);

    D3D11_RASTERIZER_DESC rsFrontDesc;
    ZeroMemory(&rsFrontDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsFrontDesc.FillMode = D3D11_FILL_SOLID;
    rsFrontDesc.CullMode = D3D11_CULL_FRONT;
    rsFrontDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&rsFrontDesc, &solidCullFrontRs);

    D3D11_RASTERIZER_DESC rsNoCullingDesc;
    ZeroMemory(&rsNoCullingDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsNoCullingDesc.FillMode = D3D11_FILL_SOLID;
    rsNoCullingDesc.CullMode = D3D11_CULL_NONE;
    rsNoCullingDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&rsNoCullingDesc, &noCullingRs);

    D3D11_RASTERIZER_DESC rsWireframeDesc;
    ZeroMemory(&rsWireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsWireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
    rsWireframeDesc.CullMode = D3D11_CULL_BACK;
    rsWireframeDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&rsWireframeDesc, &wireframeRs);
}

void D3D11Device::InitBlendStates()
{
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    DX_CALL(device->CreateBlendState(&blendDesc, &alphaBlendEnable), "Couldn't create blend state");

    // Set no blend for opaque objects
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    DX_CALL(device->CreateBlendState(&blendDesc, &alphaBlendDisable), "Couldn't create blend state");
}

void D3D11Device::InitPostProcessUAV()
{
    // Create Texture2D
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = resolutionSize.x;
    textureDesc.Height = resolutionSize.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &postProcessTexture), DXE_ERROR_CREATING_TEXTURE);

    // Create SamplerState
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    DX_CALL(device->CreateSamplerState(&samplerDesc, &postProcessSamplerState), DXE_ERROR_CREATE_SAMPLER_STATE);

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    shaderDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderDesc.Texture2D.MipLevels = 1;
    shaderDesc.Texture2D.MostDetailedMip = 0;

    DX_CALL(device->CreateShaderResourceView(postProcessTexture, &shaderDesc, &postProcessSRV), DXE_ERROR_CREATING_SHADER_VIEW);

    // Create unordered access view
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    uavDesc.Texture2D.MipSlice = 0;

    DX_CALL(device->CreateUnorderedAccessView(postProcessTexture, &uavDesc, &postProcessUAV), DXE_ERREURCREATIONRENDERTARGET);

    // Create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    rtvDesc.Texture2D.MipSlice = 0;

    DX_CALL(device->CreateRenderTargetView(postProcessTexture, &rtvDesc, &postProcessRTV), DXE_ERREURCREATIONRENDERTARGET);
}

void D3D11Device::SetViewPort(const DirectX::XMINT2& size) const
{
    const auto& [w, h] = size;
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(w);
    vp.Height = static_cast<FLOAT>(h);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    immediateContext->RSSetViewports(1, &vp);
}

}
