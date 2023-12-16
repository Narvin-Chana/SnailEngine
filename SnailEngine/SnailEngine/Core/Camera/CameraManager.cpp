#include "stdafx.h"
#include "CameraManager.h"

namespace Snail
{
void CameraManager::AddCameraPerspective(const Transform& initialTransform, const float fov, const float nearPl, const float farPl)
{
    LOGF("Added perspective camera [{}]\n " "\twith transform = {{ {} }}\n" "\tfov = {}\n" "\tnearPlane = {}\n" "\tfarPlane = {}",
         cameras.size(),
         initialTransform,
         fov,
         nearPl,
         farPl);
    std::unique_ptr<Camera> perspCam = std::make_unique<Camera>(initialTransform, nearPl, farPl, Camera::CameraProjectionType::CAM_PERSPECTIVE);

    // This cast should be safe, assert just in case
    assert(dynamic_cast<PerspectiveProjection*>(perspCam->GetProjection()) != nullptr);
    reinterpret_cast<PerspectiveProjection*>(perspCam->GetProjection())->fov = fov;

    cameras.push_back(std::move(perspCam));
}

void CameraManager::AddCameraOrtho(const Transform& initialTransform, const float nearPl, const float farPl)
{
    LOGF("Added orthographic camera [{}]\n " "\twith transform = {{ {} }}\n" "\tnearPlane = {}\n" "\tfarPlane = {}",
         cameras.size(),
         initialTransform,
         nearPl,
         farPl);
    cameras.push_back(std::make_unique<Camera>(initialTransform, nearPl, farPl, Camera::CameraProjectionType::CAM_ORTHOGRAPHIC));
}

Camera* CameraManager::ChangeCamera(const int cameraNb)
{
    ChangeViewCamera(cameraNb);
    return ChangeControlledCamera(cameraNb);
}

Camera* CameraManager::ChangeViewCamera(const int cameraNb)
{
    if (cameras.empty())
        AddCameraPerspective({});
    currentCamera = std::max(0, std::min(cameraNb, (static_cast<int>(cameras.size() - 1))));

    LOGF("Changed to camera {}", cameraNb);
    return cameras[currentCamera].get();
}

Camera* CameraManager::ChangeControlledCamera(const int nb)
{
    if (cameras.empty())
        AddCameraPerspective({});
    currentControlledCamera = std::max(0, std::min(nb, (static_cast<int>(cameras.size() - 1))));
    return cameras[currentControlledCamera].get();
}

Camera* CameraManager::GetCurrentCamera() const
{
    if (cameras.empty())
        return nullptr;
    return cameras[currentCamera].get();
}

Camera* CameraManager::GetControlledCamera() const
{
    if (cameras.empty())
        return nullptr;
    return cameras[currentControlledCamera].get();
}

Camera* CameraManager::GetCamera(const int nb) const
{
    if (cameras.size() - 1 < nb) { return nullptr; }
    return cameras[nb].get();
}

const Camera* CameraManager::GetFirstPerspectiveCamera() const noexcept
{
    if (const Camera* cam = GetCurrentCamera(); cam->IsPerspectiveCamera()) { return cam; }

    if (const auto it = std::ranges::find_if(cameras, [](const auto& cam){ return cam->IsPerspectiveCamera(); }); *it)
    {
        return it->get();
    }

    return nullptr;
}

void CameraManager::SetFov(const float fov)
{
    const Camera* cam = GetCurrentCamera();
    assert(dynamic_cast<PerspectiveProjection*>(cam->GetProjection()) != nullptr);
    reinterpret_cast<PerspectiveProjection*>(cam->GetProjection())->fov = DirectX::XM_PI/180.f * fov;
}
float CameraManager::GetFov() const
{
    const Camera* cam = GetCurrentCamera();
    assert(dynamic_cast<PerspectiveProjection*>(cam->GetProjection()) != nullptr);
    return reinterpret_cast<PerspectiveProjection*>(cam->GetProjection())->fov / DirectX::XM_PI * 180.f;
}

void CameraManager::Cleanup() { cameras.clear(); }

CameraManager::CameraManager()
    : currentCamera{0} { }

}
