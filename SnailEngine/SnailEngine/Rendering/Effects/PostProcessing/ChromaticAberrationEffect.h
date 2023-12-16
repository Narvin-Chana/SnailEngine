#pragma once
#include "PostProcessEffect.h"

namespace Snail
{

struct ChromaticAberrationParameters
{
    Vector3 rgbOffset = { 0.5f, 0.000f, -0.5f };
    float intensity = 30;
};

class ChromaticAberrationEffect : public PostProcessEffect<ChromaticAberrationParameters>
{
public:
    ChromaticAberrationEffect();

    using PostProcessEffect::RenderEffect;
    void RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output) override;

    void RenderImGui() override;
};

}
