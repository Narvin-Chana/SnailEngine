#pragma once
#include "PostProcessEffect.h"
#include "BlurEffect.h"

#include "Util/Util.h"

namespace Snail
{
class Texture2D;

struct SSAOParameters
{
    DX_ALIGN Matrix invProjMat;
    DX_ALIGN Matrix projMat;
    DX_ALIGN Matrix invViewMat;
    std::array<Vector4, 64> samples;
    DirectX::XMINT2 screenSize;
    int kernelSize = 64;
    float radius = 0.5f;
    float bias = 0.025f;
    float power = 4;
};

class SSAOEffect : public PostProcessEffect<SSAOParameters>
{
    std::unique_ptr<Texture2D> noiseTexture{};
    ID3D11Texture2D* outputTexture{};
    ID3D11UnorderedAccessView* outputUAV{};
    ID3D11ShaderResourceView* outputSRV{};
    BlurEffect blur;

    void CalculateRotationNoiseTexture();
    void CalculateNoiseData();
    void ResizeTexture(long width, long height);

public:
    SSAOEffect(D3D11Device* device);
    ~SSAOEffect() override;

    D3D11Device* renderDevice;

    ID3D11Texture2D* GetSSAOTexture();
    ID3D11UnorderedAccessView* GetSSAOUAV();
    ID3D11ShaderResourceView* GetSSAOSRV();

    void RenderEffect(D3D11Device* renderDevice) override;
    void RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output) override;

    void RenderImGui() override;
    void Resize(long width, long height);
};

}
