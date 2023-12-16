#pragma once
#include <vector>
#include <memory>

#include "Util/Singleton.h"
#include "Camera.h"

#include "Core/Math/Transform.h"
#include "Projections/OrthographicProjection.h"
#include "Projections/PerspectiveProjection.h"

namespace Snail
{
class CameraManager
{
    friend Singleton;

public:
    void AddCameraPerspective(const Transform& initialTransform, float fov = PerspectiveProjection::DEFAULT_FOV, float nearPl = PerspectiveProjection::DEFAULT_NEAR_PLANE, float farPl = PerspectiveProjection::DEFAULT_FAR_PLANE);
    void AddCameraOrtho(const Transform& initialTransform, float nearPl = OrthographicProjection::DEFAULT_NEAR_PLANE, float farPl = OrthographicProjection::DEFAULT_FAR_PLANE);

    Camera* ChangeCamera(int cameraNb);
    Camera* ChangeViewCamera(int cameraNb);
    Camera* ChangeControlledCamera(int nb);
    Camera* GetCurrentCamera() const;
    Camera* GetControlledCamera() const;
    Camera* GetCamera(int nb) const;
    const Camera* GetFirstPerspectiveCamera() const noexcept;

    void SetFov(float fov);
    float GetFov() const;

    void Cleanup();
    CameraManager();

private:
    std::vector<std::unique_ptr<Camera>> cameras;
    int currentCamera;
    int currentControlledCamera = -1;
};
}
