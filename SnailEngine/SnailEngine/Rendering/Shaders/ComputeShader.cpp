#include "stdafx.h"
#include "ComputeShader.h"

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/Resource.h"

namespace Snail
{
ComputeShader::ComputeShader(const std::wstring& filename, const std::unordered_set<std::string>& defines)
    : defines(defines)
{
    const auto device = WindowsEngine::GetInstance().GetRenderDevice()->GetD3DDevice();

    // Compilation et chargement du compute shader
    ID3DBlob* pCSBlob = nullptr;
    ID3DBlob* pCSErrorBlob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    std::vector<D3D_SHADER_MACRO> macros;
    std::ranges::transform(defines, std::back_inserter(macros), [](const std::string& define)-> D3D_SHADER_MACRO {return { define.c_str(), nullptr }; });

    // this acts as a sentinel value to indicate the end of the macros
    macros.emplace_back(nullptr, nullptr);

    const HRESULT hr = D3DCompileFromFile(filename.c_str(), macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0",
        flags, 0, &pCSBlob, &pCSErrorBlob);

    DX_COMPILE_SHADER(pCSErrorBlob, hr, DXE_FICHIER_CS);

    DX_CALL(device->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &internalShader),
            DXE_CREATION_CS);

    Reflect(pCSBlob);
    pCSBlob->Release();
}

ComputeShader::~ComputeShader()
{
    DX_RELEASE(internalShader);
}

void ComputeShader::Bind()
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    context->CSSetShader(internalShader, nullptr, 0);
    context->CSSetConstantBuffers(0, static_cast<UINT>(constantBuffers.size()), constantBuffers.data());
}

void ComputeShader::Execute(const int threadGroupCountX, const int threadGroupCountY, const int threadGroupCountZ)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void ComputeShader::Unbind()
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();

    ID3D11UnorderedAccessView* ppUAViewnullptr[1] = {nullptr};
    device->GetImmediateContext()->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
}

void ComputeShader::BindComputedUAV(ID3D11UnorderedAccessView* uav)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
}

void ComputeShader::BindSRVAndSampler(const int slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* samplerState)
{
    BindSRV(slot, srv);
    BindSampler(slot, samplerState);
}

void ComputeShader::BindSampler(const int slot, ID3D11SamplerState* samplerState)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->CSSetSamplers(slot, 1, &samplerState);
}

void ComputeShader::BindSRV(const int slot, ID3D11ShaderResourceView* srv)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->CSSetShaderResources(slot, 1, &srv);
}

void ComputeShader::UnbindSRV(int slot)
{
    static D3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice();
    ID3D11ShaderResourceView* srv[] = { nullptr };
    device->GetImmediateContext()->CSSetShaderResources(slot, 1, srv);
}

}
