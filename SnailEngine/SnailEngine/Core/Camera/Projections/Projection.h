#pragma once

namespace Snail
{
class Camera;

struct Projection
{
    static constexpr float DEFAULT_NEAR_PLANE = 0.5f;
    static constexpr float DEFAULT_FAR_PLANE = 500.0f;

    float nearPlane;
    float farPlane;
    float aspectRatio = 16.0f / 9.0f;

    Projection()
    {
        nearPlane = DEFAULT_NEAR_PLANE;
        farPlane = DEFAULT_FAR_PLANE;
    }

    virtual ~Projection() = default;
    virtual Matrix SetupProjectionMatrix(long screenWidth, long screenHeight) = 0;
    virtual Matrix GetProjectionMatrix() = 0;
    virtual Matrix GetProjectionMatrix(float nearPl, float farPl) = 0;
    virtual bool RenderImGui(Camera*)
    {
#ifdef _IMGUI_
        return true;
#else
        return false;
#endif
    };
};

}
