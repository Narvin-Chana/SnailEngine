#pragma once
#include <mutex>
#include "Core/RendererModule.h"

// This should be used where context writes are called
inline std::mutex DeviceMutex;

namespace Snail
{
class Device
{
#ifdef _IMGUI_
    friend RendererModule;
#endif

protected:
    DirectX::XMINT2 resolutionSize{};
    bool VSyncEnabled = false;

public:
    // Constantes pour mode fenêtré ou plein écran
    enum class DisplayMode
    {
        WINDOWED,
        FULL_SCREEN
    };

    virtual ~Device() = default;
    virtual void InitGBuffer() = 0;
    virtual void Present() { PresentSpecific(); };

    const DirectX::XMINT2& GetResolutionSize() const noexcept { return resolutionSize; }

    virtual void SetResolution(long width, long height) = 0;
    virtual void SetDisplayMode(DisplayMode) = 0;
    virtual void PresentSpecific() = 0;
    // Rendering
    virtual void PrepareDeferredDraw() = 0;
    virtual void PrepareDecalDraw() = 0;
    virtual void PrepareVolumetricDraw() = 0;
    virtual void PrepareLightingDraw() = 0;
    virtual void PrepareSkyboxDraw() = 0;
    virtual void PreparePostProcessDraw() = 0;
    virtual void DrawIndexedInstanced(
        int indexCountPerInstance,
        int instanceCount,
        int startIndexLocation = 0,
        int baseVertexLocation = 0,
        int startInstanceLocation = 0) = 0;
    virtual void DrawIndexed(unsigned int vertexCount, unsigned int startVertex = 0) = 0;
    virtual void Draw(unsigned int vertexCount) = 0;
};
}
