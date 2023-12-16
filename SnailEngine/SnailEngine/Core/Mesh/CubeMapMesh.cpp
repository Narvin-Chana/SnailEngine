#include "stdafx.h"
#include "CubeMapMesh.h"

#include "Core/WindowsEngine.h"
#include "Core/Camera/CameraManager.h"

namespace Snail
{
CubeMapMesh::CubeMapMesh()
    : cubeMapTexture(nullptr)
    , data{D3D11Buffer::CreateConstantBuffer<Matrix>()}
    , volumetricDensityFactor(D3D11Buffer::CreateConstantBuffer<float>())
{
    indexes = {
        0, 1, 2, // front
        1, 3, 2, // front
        4, 6, 5, // back
        5, 6, 7, // back
        2, 3, 6, // down
        6, 3, 7, // down
        0, 4, 1, // up
        4, 5, 1, // up
        2, 6, 0, // left
        0, 6, 4, // left
        1, 5, 3, // right
        5, 7, 3 // right
    };

    constexpr float dx = 1;
    constexpr float dy = 1;
    constexpr float dz = 1;

    // Les points
    Vector3 point[8] = {
        Vector3(-dx / 2, dy / 2, -dz / 2), Vector3(dx / 2, dy / 2, -dz / 2), Vector3(-dx / 2, -dy / 2, -dz / 2), Vector3(dx / 2, -dy / 2, -dz / 2),
        Vector3(-dx / 2, dy / 2, dz / 2), Vector3(dx / 2, dy / 2, dz / 2), Vector3(-dx / 2, -dy / 2, dz / 2), Vector3(dx / 2, -dy / 2, dz / 2),
    };

    for (int i = 0; i < 8; ++i)
    {
        vertices.emplace_back(point[i], point[i]);
    }
}

void CubeMapMesh::InitShaders()
{
    effectsShader = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/CubeSkybox.fx", DEFAULT_ELEMENT_LAYOUT, DEFAULT_ELEMENT_COUNT);
}

void CubeMapMesh::BindBuffers(const D3D11Buffer*)
{
    const Camera* targetCam = WindowsEngine::GetModule<CameraManager>().GetFirstPerspectiveCamera();

    if (!targetCam) { return; }

    const Matrix proj = targetCam->GetProjectionMatrix();

    Quaternion camRot;
    targetCam->GetTransform().rotation.Inverse(camRot);
    const Matrix mat = Matrix::CreateFromQuaternion(camRot);
    data.UpdateData((mat * proj).Transpose());

    effectsShader->SetConstantBuffer("TransformMatrixes", data.GetBuffer());

    const Scene* scene = WindowsEngine::GetScene();
    volumetricDensityFactor.UpdateData(scene->GetVolumetricFactor());
    effectsShader->SetConstantBuffer("VolumetricDensityFactor", volumetricDensityFactor.GetBuffer());
}

void CubeMapMesh::BindTextures(const SubMesh&) const
{
    static RendererModule& rm = WindowsEngine::GetModule<RendererModule>();

    assert(cubeMapTexture);
    effectsShader->BindTexture("SkyboxTexture", cubeMapTexture);
    effectsShader->BindShaderResourceView("VolumetricAccumulation", rm.GetVolumetricLighting()->GetVolumetricAccumulationBuffer()->GetShaderResourceView());
}
}
