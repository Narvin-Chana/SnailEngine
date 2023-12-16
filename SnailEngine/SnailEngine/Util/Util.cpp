#include "stdafx.h"
#include "Util/Util.h"

#include "Entities/Entity.h"
#include <DirectXCollision.h>

#include "Core/WindowsEngine.h"
#include "Entities/GrassGenerator.h"

namespace Snail
{
std::vector<Vector3> GetFrustumCornersWorldSpace(const Matrix& viewProj)
{
    const auto inv = viewProj.Invert();

    std::vector<Vector3> frustumCorners;
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z)
            {
                const Vector3 v{2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f};
                frustumCorners.push_back(Vector3::Transform(v, inv));
            }
        }
    }

    return frustumCorners;
}

bool IsInFrustum(const DirectX::BoundingFrustum& frustum, Entity& entity)
{
    const DirectX::BoundingBox& boundingBox = entity.GetBoundingBox();
    return IsInFrustum(frustum, boundingBox);
}

bool IsInFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::BoundingBox& boundingBox)
{
    const bool collides = boundingBox.Intersects(frustum);

#ifdef _DEBUG
    static RendererModule& renderer = WindowsEngine::GetModule<RendererModule>();
    if (renderer.drawBoundingBoxes)
        renderer.DrawBoundingBox(boundingBox, collides ? Color(0, 1, 0) : Color(1, 0, 0));
#endif

    return collides;
}

bool IsInFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::BoundingOrientedBox& boundingOrientedBox)
{
    const bool collides = boundingOrientedBox.Intersects(frustum);

#ifdef _DEBUG
    static RendererModule& renderer = WindowsEngine::GetModule<RendererModule>();
    if (renderer.drawBoundingBoxes)
        renderer.DrawBoundingBox(boundingOrientedBox, collides ? Color(0, 1, 0) : Color(1, 0, 0));
#endif

    return collides;
}

bool IsGrassPatchInFrustum(const DirectX::BoundingFrustum& frustum, const GrassGenerator& grassPatch)
{
    const DirectX::BoundingOrientedBox& boundingBox = grassPatch.GetBoundingBox();

    return IsInFrustum(frustum, boundingBox);
}

DirectX::BoundingFrustum GetFrustumFromCamera(const Camera* projectionCamera)
{
    assert(projectionCamera->IsPerspectiveCamera());

    DirectX::BoundingFrustum frustum = DirectX::BoundingFrustum(projectionCamera->GetProjectionMatrix(projectionCamera->GetNearPlane(), projectionCamera->GetFarPlane()), false);
    // Move frustum to camera's transform location
    auto [position, rotation, scale] = projectionCamera->GetTransform();
    rotation.Normalize();

    frustum.Transform(frustum, scale.x, rotation, position);

    // this should alway be true in theory
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&frustum.Orientation);
    assert(DirectX::Internal::XMQuaternionIsUnit(quat));
    return frustum;
}
}
