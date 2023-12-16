#pragma once
#include "PostProcessEffect.h"

namespace Snail
{
    class Texture2D;

    struct BlurParameters
    {
        int kernelSize = 4;
    };

    class BlurEffect : public PostProcessEffect<BlurParameters>
    {
        std::unique_ptr<Texture2D> noiseTexture{};

    public:
        BlurEffect();

        using PostProcessEffect::RenderEffect;
        void RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output) override;

        void RenderImGui() override;
    };

}
