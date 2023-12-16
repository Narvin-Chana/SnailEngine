#include "stdafx.h"
#include "DirectionalShadowMap.h"

#include "Core/WindowsEngine.h"
#include "Entities/Entity.h"
#include "Entities/Terrain.h"
#include "Rendering/MeshVertex.h"
#include "Rendering/Texture2D.h"

namespace Snail
{
DirectionalShadowMap::DirectionalShadowMap(D3D11Device* device)
    : renderDevice(device)
    , vsShader{L"SnailEngine/Shaders/WriteToDepth.vs.hlsl", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT}
    , psShader{L"SnailEngine/Shaders/WriteToDepth.ps.hlsl"}
    , cascadeShadowBuffer{D3D11Buffer::CreateConstantBuffer<CascadeShadowData[CASCADE_COUNT * SceneData::MAX_DIR_LIGHTS]>()}
    , viewProjBuffer{D3D11Buffer::CreateConstantBuffer<Matrix>()}
{
    D3D11_DEPTH_STENCIL_VIEW_DESC stencilDesc;
    renderDevice->GetDepthStencilView()->GetDesc(&stencilDesc);

    D3D11_TEXTURE2D_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ArraySize = CASCADE_COUNT * SceneData::MAX_DIR_LIGHTS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Width = 2048;
    desc.Height = desc.Width;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    shadowMap = std::make_unique<Texture2D>(desc, DXGI_FORMAT_R32_FLOAT, false);

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    shadowMap->SetSampler(samplerDesc);

    depthStencilView.resize(desc.ArraySize);
    for (uint32_t i = 0; i < desc.ArraySize; ++i)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
        ZeroMemory(&descDSView, sizeof(descDSView));
        descDSView.Format = DXGI_FORMAT_D32_FLOAT;
        descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        descDSView.Texture2DArray.ArraySize = 1;
        descDSView.Texture2DArray.FirstArraySlice = i;
        descDSView.Texture2DArray.MipSlice = 0;
        DX_CALL(renderDevice->GetD3DDevice()->CreateDepthStencilView( shadowMap->GetRawTexture(), &descDSView, &depthStencilView[i] ),
                "Could not create depth stencil view for shadow map");
    }

    viewport.Width = static_cast<FLOAT>(desc.Width);
    viewport.Height = static_cast<FLOAT>(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;


    D3D11_RASTERIZER_DESC rsNoCullingDesc;
    ZeroMemory(&rsNoCullingDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsNoCullingDesc.FillMode = D3D11_FILL_SOLID;
    rsNoCullingDesc.CullMode = D3D11_CULL_NONE;

    // Magic heuristic values found by trial and error.
    // For more info, read this: https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage-depth-bias
    constexpr int depthBias = -1;
    constexpr float depthClamp = 0;
    constexpr float slopeBias = 0;
    rsNoCullingDesc.DepthBias = depthBias;
    rsNoCullingDesc.SlopeScaledDepthBias = slopeBias;
    rsNoCullingDesc.DepthBiasClamp = depthClamp;
    DX_CALL(device->GetD3DDevice()->CreateRasterizerState(&rsNoCullingDesc, &shadowRS), "Error creating CSM rasterizerState");
}

DirectionalShadowMap::~DirectionalShadowMap()
{
    for (auto* view : depthStencilView)
        DX_RELEASE(view);

    DX_RELEASE(shadowRS);
}

Texture2D* DirectionalShadowMap::GetDepthTexture() const
{
    return shadowMap.get();
}

std::pair<Matrix, DirectX::BoundingOrientedBox> DirectionalShadowMap::GetLightSpaceMatrix(const DirectionalLight& light, const int cascadeId)
{
    static auto& cameraManager = WindowsEngine::GetModule<CameraManager>();
    const Camera* camera = cameraManager.GetFirstPerspectiveCamera();
#ifdef _DEBUG
    if (drawCascades)
        camera = cameraManager.GetCamera(0);
#endif

    auto& [n, f] = cascades[cascadeId];

    if (n == f)
        return { Matrix::Identity, DirectX::BoundingOrientedBox() };

    const std::vector frustrumPoints = GetFrustumCornersWorldSpace(camera->GetViewMatrix() * camera->GetProjectionMatrix(n, f));
#ifdef _DEBUG
    if (drawCascades)
    {
        static auto& renderer = WindowsEngine::GetModule<RendererModule>();
        DirectX::BoundingFrustum frustum{camera->GetProjectionMatrix(n, f)};
        frustum.Transform(frustum, camera->GetViewMatrix().Invert());
        renderer.DrawFrustum(frustum, Color{ 1,0,1 });
    }
#endif

    Vector3 center{};
    for (const Vector3& v : frustrumPoints)
        center += v;
    center /= static_cast<float>(frustrumPoints.size());


    Vector3 cross = light.Direction;
    cross.x += 1;
    cross.z -= 1;

    Vector3 result;
    light.Direction.Cross(cross, result);
    result.Normalize();

    const Matrix viewMatrix = Matrix::CreateLookAt(center, center + light.Direction, result);

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const Vector3& v : frustrumPoints)
    {
        const Vector3 trf = Vector3::Transform(v, viewMatrix);
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Extend the cascade to have shadows of farther objects
    minZ -= 20;
    maxZ += 20;

    auto inv = viewMatrix.Invert();

    DirectX::BoundingOrientedBox boundingBox{ {}, { maxX - minX, maxY - minY, maxZ - minZ }, Quaternion::Identity };
    boundingBox.Transform(boundingBox, inv);

#ifdef _DEBUG
    if (drawCascades)
    {
        constexpr Color blue{0,0,1};
        static auto& renderer = WindowsEngine::GetModule<RendererModule>();
        renderer.DrawBoundingBox(boundingBox, blue);

        constexpr Color red{1,0,0};
        renderer.DrawLine({ Vector3::Transform({ 1, 0, 0 }, inv), red }, { Vector3::Transform({ -1, 0, 0 }, inv), red });
        renderer.DrawLine({ Vector3::Transform({ 0, 1, 0 }, inv), red }, { Vector3::Transform({ 0, -1, 0 }, inv), red });
        renderer.DrawLine({ Vector3::Transform({ 0, 0, 1 }, inv), red }, { Vector3::Transform({ 0, 0, -1 }, inv), red });
    }
#endif
    const Matrix projection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, maxZ, minZ);
    return { viewMatrix * projection, boundingBox };
}

const D3D11Buffer& DirectionalShadowMap::GetViewProjBuffer(const std::vector<DirectionalLight>& lights)
{
    const Camera* camera = WindowsEngine::GetModule<CameraManager>().GetControlledCamera();

    CascadeShadowData data[CASCADE_COUNT * SceneData::MAX_DIR_LIGHTS];

    for (int lightI = 0; lightI < lights.size(); ++lightI)
    {
        if (!lights[lightI].castsShadows)
            continue;

        for (int i = 0; i < CASCADE_COUNT; ++i)
        {
            data[lightI * CASCADE_COUNT + i].cascadeBound = Vector2{
                cascades[i].first.value_or(camera->GetNearPlane()),
                cascades[i].second.value_or(camera->GetFarPlane())
            };
            data[lightI * CASCADE_COUNT + i].matrix = cascadeInfo[i].first.Transpose();
        }
    }

    cascadeShadowBuffer.UpdateData(data);
    return cascadeShadowBuffer;
}

void DirectionalShadowMap::Render(const FixedVector<DirectionalLight, SceneData::MAX_DIR_LIGHTS>& lights)
{
    static auto& engine = WindowsEngine::GetInstance();
    static auto& renderer = engine.GetModule<RendererModule>();
    static auto* device = engine.GetRenderDevice();
    const auto* scene = engine.GetScene();
    auto* context = renderDevice->GetImmediateContext();

    // Unbind RTVs
    // Have to pass in nullpointing pointer in order to unbind RTV & UAV...thanks DX11...
    ID3D11RenderTargetView* nullRTV = nullptr;
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->OMSetRenderTargetsAndUnorderedAccessViews(1, &nullRTV, nullptr, 1, 1, &nullUAV, nullptr);

    // Repeat this for each section of the cascade
    for (int lightI = 0; lightI < lights.size(); ++lightI)
    {
        if (!lights[lightI].castsShadows)
            continue;

        for (int i = 0; i < CASCADE_COUNT; ++i)
        {
            const int dvIndex = lightI * CASCADE_COUNT + i;

            context->OMSetRenderTargets(0, nullptr, depthStencilView[dvIndex]);
            context->ClearDepthStencilView(depthStencilView[dvIndex], D3D11_CLEAR_DEPTH, 0, 0);
            context->RSSetViewports(1, &viewport);

            cascadeInfo[i] = GetLightSpaceMatrix(lights[lightI], i);

            ShadowDrawContext ctx{ &renderer, device, cascadeInfo[i].second };

            for (Entity* entity : scene->GetEntities())
            {
                // Doesnt work for nested objects that do their own culling
                if (!entity->ShouldCastShadows())
                    continue;

                viewProjBuffer.UpdateData(cascadeInfo[i].first.Transpose());
                vsShader.SetConstantBuffer(0, viewProjBuffer.GetBuffer());

                vsShader.Bind();
                psShader.Bind();

                device->GetImmediateContext()->RSSetState(shadowRS);
                entity->Draw(ctx);
            }

            renderer.DrawMeshesGeometry();

            for (GrassGenerator* grassPatch : scene->GetGrassPatches())
            {
                if (!cascadeInfo[i].second.Intersects(grassPatch->GetBoundingBox()))
                    continue;

                viewProjBuffer.UpdateData(cascadeInfo[i].first.Transpose());
                device->GetImmediateContext()->RSSetState(shadowRS);

                grassPatch->DrawShadows(viewProjBuffer);
            }
        }
    }
    renderDevice->ResetRenderTarget();
    renderDevice->ResetViewPort();
}

void DirectionalShadowMap::RenderImGui()
{
#ifdef _IMGUI_
    using namespace std::literals;
    if (ImGui::CollapsingHeader("Directional ShadowMap"))
    {
        const Camera* camera = WindowsEngine::GetModule<CameraManager>().GetControlledCamera();
        ImGui::Checkbox("Draw cascades", &drawCascades);
        for (int i = 0; i < cascades.size(); ++i)
        {
            auto& [lowerBound, upperBound] = cascades[i];

            if (!lowerBound)
            {
                ImGui::Text("near plane");
                ImGui::SameLine();
                ImGui::DragFloat(("##Upper_bound_"s + std::to_string(i)).c_str(), &*upperBound, 0.5f,
                    camera->GetNearPlane(),
                    camera->GetFarPlane());
            }
            else if (!upperBound)
            {
                ImGui::DragFloat(("##Lower_bound_"s + std::to_string(i)).c_str(), &*lowerBound, 0.5f,
                    camera->GetNearPlane(),
                    camera->GetFarPlane());
                ImGui::SameLine();
                ImGui::Text("far plane");
            }
            else
            {
                float vals[2] = { *lowerBound, *upperBound };
                ImGui::DragFloat2(("##Upper_bound_"s + std::to_string(i)).c_str(), vals, 0.5f,
                    camera->GetNearPlane(),
                    camera->GetFarPlane());

                *lowerBound = vals[0];
                *upperBound = vals[1];
            }
        }

        ImGui::Image(shadowMap->GetShaderResourceView(), ImVec2(100,100), {}, {1, -1});
    }
#endif
}
ShadowDrawContext::ShadowDrawContext(RendererModule* rm, D3D11Device* dev, const DirectX::BoundingOrientedBox& obb)
    : DrawContext(rm, dev)
    , sumObb{obb}
{
}
bool ShadowDrawContext::ShouldBeCulled(const DirectX::BoundingBox& bb)
{
    return !sumObb.Intersects(bb);
}
bool ShadowDrawContext::ShouldBeCulled(const DirectX::BoundingOrientedBox& obb)
{
    return !sumObb.Intersects(obb);
}
bool ShadowDrawContext::ShouldBeCulled(const DirectX::BoundingSphere& bs)
{
    return !sumObb.Intersects(bs);
}
}
