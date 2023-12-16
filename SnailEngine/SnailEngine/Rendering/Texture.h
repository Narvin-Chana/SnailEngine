#pragma once
#include <string>

struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace Snail
{

class Image;

class Texture
{
protected:
    ID3D11Texture2D* rawTexture{};

    ID3D11ShaderResourceView* shaderResourceView{};
    ID3D11SamplerState* samplerState{};

    virtual void InitResources(const Image& image) = 0;
    virtual void InitSampler();

public:
    Texture() = default;
    virtual ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    ID3D11ShaderResourceView* GetShaderResourceView() const noexcept { return shaderResourceView; }
    ID3D11SamplerState* GetSamplerState() const noexcept { return samplerState; }
    ID3D11Texture2D* GetRawTexture() const noexcept { return rawTexture; }
    void SetSampler(const D3D11_SAMPLER_DESC& samplerDesc);

    virtual void RenderImGui();

    Vector2 GetDimensions() const;
};

}
