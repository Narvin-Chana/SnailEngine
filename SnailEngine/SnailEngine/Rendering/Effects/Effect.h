#pragma once

namespace Snail
{
class D3D11Device;

class Effect
{
protected:
    bool isActive = false;

public:
    virtual ~Effect() = default;
    virtual void Update(float dt) = 0;

    bool IsActive() const noexcept;
    void SetActive(bool isEffectActive);

    virtual void RenderImGui() = 0;
};
}
