#pragma once
#include "PostProcessEffect.h"

#include "Util/Util.h"

namespace Snail
{

struct VignetteParameters
{
    Color color = Color{ 1,1,1 };
    float radiusDivisionFactor = 1;
    float intensity = 0.5f;
};

class VignetteEffect : public PostProcessEffect<VignetteParameters>
{
public:
    VignetteEffect();

    using PostProcessEffect::RenderEffect;
    void RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output) override;

    void RenderImGui() override;
};

}
