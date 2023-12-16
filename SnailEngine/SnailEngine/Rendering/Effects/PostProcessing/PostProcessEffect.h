#pragma once

#include "Rendering/D3D11Device.h"
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/Effects/Effect.h"
#include "Rendering/Shaders/ComputeShader.h"

namespace Snail
{
class D3D11Device;

template <class T>
class PostProcessEffect : public Effect
{
protected:
    std::wstring effectShaderFile;
    std::unique_ptr<ComputeShader> computeShader = nullptr;

    T data{};
    D3D11Buffer dataBuffer;

public:
    PostProcessEffect(const std::wstring& effectShaderPath);
    ~PostProcessEffect() override = default;

    void Update(float) override {}
    virtual void RenderEffect(D3D11Device* renderDevice, ID3D11Texture2D* tex, ID3D11UnorderedAccessView* output) = 0;
    virtual void RenderEffect(D3D11Device* renderDevice)
    {
        RenderEffect(renderDevice, renderDevice->GetPostProcessTexture(), renderDevice->GetPostProcessUAV());
    };

    T GetData() const noexcept;
    void SetData(const T& newData);

    void ReloadShader();
};

template <class T>
PostProcessEffect<T>::PostProcessEffect(const std::wstring& effectShaderPath)
    : effectShaderFile(effectShaderPath)
    , computeShader(std::make_unique<ComputeShader>(effectShaderPath))
    , dataBuffer(D3D11Buffer::CreateConstantBuffer<T>())
{
    dataBuffer.UpdateData(data);
}

template <class T>
T PostProcessEffect<T>::GetData() const noexcept
{
    return data;
}

template <class T>
void PostProcessEffect<T>::SetData(const T& newData)
{
    data = newData;
    dataBuffer.UpdateData(data);
}

template <class T>
void PostProcessEffect<T>::ReloadShader()
{
    computeShader.reset(new ComputeShader(effectShaderFile));
}
}
