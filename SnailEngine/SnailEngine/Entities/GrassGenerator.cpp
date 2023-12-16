#include "stdafx.h"
#include "GrassGenerator.h"

#include "Terrain.h"
#include "Core/WindowsEngine.h"
#include "Rendering/D3D11Device.h"
#include "Rendering/InputAssembler.h"
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/Buffers/StructuredBuffer.h"

namespace Snail
{

void GrassGenerator::DoDrawCall() const
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    InputAssembler::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    InputAssembler::SetVertexBuffer(vertexBuffer, sizeof(GrassVertex), 0);
    InputAssembler::SetIndexBuffer(indexBuffer);
    // Could draw indirect to specify the correct amount depending on grass coverage texture
    device->DrawIndexedInstanced(21, DispatchThreadCount[0] * DispatchThreadCount[1] * regionCount[0] * regionCount[1]);
}

GrassGenerator::GrassGenerator(
    float grassDensityScale,
    std::array<uint32_t, 2> regionCount,
    Transform grassPatchPosition,
    Texture2D* sampleTexture)
    : grassComputeConstantBuffer(D3D11Buffer::CreateConstantBuffer<GrassComputeParams>())
    , grassEffectsConstantBuffer(D3D11Buffer::CreateConstantBuffer<GrassEffectsParams>())
    , indexBuffer(D3D11_BIND_INDEX_BUFFER)
    , vertexBuffer(D3D11_BIND_VERTEX_BUFFER)
    , sampleTexture(sampleTexture)
    , regionCount(regionCount)
    , grassPatchPosition(grassPatchPosition)
    , grassDensityScale(grassDensityScale)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();

    // Instantiate with garbage, will be filled in correctly by ComputeShader
    std::vector<GrassInstancedData> gid;
    gid.resize(BladeCountPerRegion * regionCount[0] * regionCount[1]);
    instancedDataBuffer = StructuredBuffer(device->GetD3DDevice(), BladeCountPerRegion * regionCount[0] * regionCount[1], gid.data());

    grassEffectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/Grass/GrassPass.fx", GrassVertex::layout, GrassVertex::elementCount);
    grassShadowsEffectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/Grass/GrassPassShadows.fx",
        GrassVertex::layout,
        GrassVertex::elementCount);

    generateGrassInstanceDataComputeShader = std::make_unique<ComputeShader>(L"SnailEngine/Shaders/Grass/GrassGeneration.cs.hlsl",
        sampleTexture ? std::unordered_set<std::string>{"SAMPLE_GRASS"} : std::unordered_set<std::string>{});
    GenerateInstanceData();

    // Our blade of grass, we declare it here to avoid IO costs for such a small .obj
    std::array vertices = {
    GrassVertex{{-0.25f, 0.0f, 0.0f}}, GrassVertex{{0.25f, 0.0f, 0.0f}}, GrassVertex{{-0.1875f, 0.5f, 0.0f}}, GrassVertex{{0.1875f, 0.5f, 0.0f}},
    GrassVertex{{-0.125f, 1.0f, 0.0f}}, GrassVertex{{0.125f, 1.0f, 0.0f}}, GrassVertex{{-0.0625f, 1.5f, 0.0f}}, GrassVertex{{0.0625f, 1.5f, 0.0f}},
    GrassVertex{{0.0f, 2.0f, 0.0f}},
    };
    // Generate uvs
    for (auto& [position, uv] : vertices)
    {
        // Recenter and scale to [0,1]
        uv = Vector2((position.x + 0.25f) * 2, position.y / 2);
    }

    vertexBuffer.UpdateData(vertices);

    constexpr std::array indices = {0, 1, 2, 1, 3, 2, 2, 3, 4, 3, 5, 4, 4, 5, 6, 5, 7, 6, 6, 7, 8};

    indexBuffer.UpdateData(indices);
}

GrassGenerator::~GrassGenerator()
{}

void GrassGenerator::Update(const float dt)
{
    totalTime += dt;
}

void GrassGenerator::UpdateGrassBuffer()
{
    const Camera* cam = WindowsEngine::GetCamera();

    GrassEffectsParams grassEffectsParams;
    grassEffectsParams.matWorldViewProj = (grassPatchPosition.GetTransformationMatrix() * cam->GetViewProjectionMatrix()).
    Transpose();
    grassEffectsParams.matWorld = grassPatchPosition.GetTransformationMatrix().Transpose();
    grassEffectsParams.matView = cam->GetViewMatrix().Transpose();
    grassEffectsParams.matViewInv = cam->GetViewMatrix().Invert();
    grassEffectsParams.time = totalTime;
    grassEffectsConstantBuffer.UpdateData(grassEffectsParams);
}

void GrassGenerator::Draw(DrawContext& ctx)
{
    if (ctx.ShouldBeCulled(GetBoundingBox()))
        return;

#ifdef _IMGUI_
    if (liveRegenerateGrassInstanceData)
    {
        // Instantiate with garbage, will be filled in correctly by ComputeShader
        std::vector<GrassInstancedData> gid;
        gid.resize(BladeCountPerRegion * regionCount[0] * regionCount[1]);
        instancedDataBuffer = StructuredBuffer(ctx.device->GetD3DDevice(), BladeCountPerRegion * regionCount[0] * regionCount[1], gid.data());

        GenerateInstanceData();
    }
#endif

    // Bind Grass Instanced data
    grassEffectsShader->BindShaderResourceView("grassInstanceData", instancedDataBuffer.srv);
    UpdateGrassBuffer();
    grassEffectsShader->SetConstantBuffer("GrassParams", grassEffectsConstantBuffer.GetBuffer());
    grassEffectsShader->Bind();

    DoDrawCall();
}

void GrassGenerator::DrawShadows(D3D11Buffer& lightMatrixBuffer)
{
    UpdateGrassBuffer();
    grassShadowsEffectsShader->BindShaderResourceView("grassInstanceData", instancedDataBuffer.srv);
    grassShadowsEffectsShader->SetConstantBuffer("GrassParams", grassEffectsConstantBuffer.GetBuffer());
    grassShadowsEffectsShader->SetConstantBuffer("TransformMatrixes", lightMatrixBuffer.GetBuffer());

    grassShadowsEffectsShader->Bind();

    DoDrawCall();
}

void GrassGenerator::GenerateInstanceData()
{
    generateGrassInstanceDataComputeShader->BindComputedUAV(instancedDataBuffer.uav);

    GrassComputeParams params;
    params.grassDensityScaleMatrix = Matrix::CreateScale(grassDensityScale).Transpose();
    params.worldPosition = grassPatchPosition.position;
    params.groupCount = regionCount;
    grassComputeConstantBuffer.UpdateData(params);
    generateGrassInstanceDataComputeShader->SetConstantBuffer("GrassParams", grassComputeConstantBuffer.GetBuffer());

    if (sampleTexture)
        generateGrassInstanceDataComputeShader->BindSRVAndSampler(0, sampleTexture->GetShaderResourceView(), sampleTexture->GetSamplerState());

    generateGrassInstanceDataComputeShader->Bind();
    generateGrassInstanceDataComputeShader->Execute(regionCount[0], regionCount[1], 1);
    generateGrassInstanceDataComputeShader->Unbind();
}

DirectX::BoundingOrientedBox GrassGenerator::GetBoundingBox() const
{
    // We fudge the numbers here since most offsets are calculated on GPU so we have no way to precisely obtain them.
    static constexpr float FudgeOffset = 2.5f;
    // Can't constexpr this since sqrt is not defined as constexpr....
    static const int BladeCountPerRegionSide = static_cast<int>(std::round(std::sqrt(BladeCountPerRegion)));

    // We do some maths stuff to apply rotation of the object with the correct origin as grass patches's origin is at their bottom-left and not center
    Vector3 grassAlignedTransform = grassPatchPosition.position + grassPatchPosition.scale * grassDensityScale * static_cast<float>(
        BladeCountPerRegionSide) * Vector3(static_cast<float>(regionCount[0]), 1, static_cast<float>(regionCount[1])) / 2.0f;
    grassAlignedTransform.y = grassPatchPosition.position.y + grassPatchPosition.scale.y;

    grassAlignedTransform = Vector3::Transform(grassAlignedTransform - grassPatchPosition.position, grassPatchPosition.rotation) + grassPatchPosition.
    position;

    const Vector3 oobScale = Vector3(grassPatchPosition.scale.x * grassDensityScale * BladeCountPerRegionSide * regionCount[0],
        grassPatchPosition.scale.y * 2,
        grassPatchPosition.scale.z * grassDensityScale * BladeCountPerRegionSide * regionCount[1]) / 2.0f + Vector3(FudgeOffset,
        FudgeOffset,
        FudgeOffset);

    return DirectX::BoundingOrientedBox(grassAlignedTransform, oobScale, grassPatchPosition.rotation);
}

void GrassGenerator::ReloadShaders()
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();

    std::lock_guard lock{ DeviceMutex };

    generateGrassInstanceDataComputeShader->Unbind();

    // Instantiate with garbage, will be filled in correctly by ComputeShader
    std::vector<GrassInstancedData> gid{};
    const int totalBladesCount = BladeCountPerRegion * regionCount[0] * regionCount[1];
    gid.resize(totalBladesCount);
    instancedDataBuffer = StructuredBuffer(device->GetD3DDevice(), totalBladesCount, gid.data());

    generateGrassInstanceDataComputeShader.reset(new ComputeShader(L"SnailEngine/Shaders/Grass/GrassGeneration.cs.hlsl",
        sampleTexture ? std::unordered_set<std::string>{"SAMPLE_GRASS"} : std::unordered_set<std::string>{}));
    GenerateInstanceData();
    grassEffectsShader->ReloadShader();
    grassShadowsEffectsShader->ReloadShader();
}

void GrassGenerator::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::PushID("GrassParams");
    {
        ImGui::Checkbox("Live Recompile Grass Instance Data", &liveRegenerateGrassInstanceData);

        ImGui::Text("Grass Density (must recompile shaders for any changes to take effect)");
        ImGui::DragFloat("##Scale", &grassDensityScale);

        ImGui::Text((std::string("Grass Blade Count Per Region: ") + std::to_string(BladeCountPerRegion)).c_str());

        ImGui::Text("Number of Regions: ");
        ImGui::DragInt2("##NumberOfRegions", reinterpret_cast<int*>(regionCount.data()), 1, 0, 1000);

        ImGui::Text((std::string("Total Number of Grass Blades: ") + std::to_string(BladeCountPerRegion * regionCount[0] * regionCount[1])).c_str());
    }
    ImGui::PopID();
    ImGui::PushID("ChunkPosition");
    {
        ImGui::SeparatorText("Chunk Position");
        Vector3 position = grassPatchPosition.position;
        Vector3 rotation = grassPatchPosition.rotation.ToEuler();
        Vector3 scale = grassPatchPosition.scale;

        ImGui::Text("Position: ");
        if (ImGui::DragFloat3("##Position", reinterpret_cast<float*>(&position))) { grassPatchPosition.position = position; }

        // This has some problems but can't be bothered to spend time fixing this.
        ImGui::Text("Rotation: ");
        if (ImGui::SliderAngle("##RotationX", &rotation.x, -89.9f, 89.9f, "x: %.0f deg") + ImGui::SliderAngle("##RotationY",
            &rotation.y,
            -89.9f,
            89.9f,
            "y: %.0f deg") + ImGui::SliderAngle("##RotationZ", &rotation.z, -89.9f, 89.9f, "z: %.0f deg"))
        {
            grassPatchPosition.rotation = Quaternion::CreateFromYawPitchRoll(rotation);
        }
        if (ImGui::Button("Reset Rotation##resetRot")) { grassPatchPosition.rotation = Quaternion::Identity; }

        ImGui::Text("Scale (per blade): ");
        if (ImGui::DragFloat3("##Scale", reinterpret_cast<float*>(&scale))) { grassPatchPosition.scale = scale; }
    }
    ImGui::PopID();

#endif
}

}
