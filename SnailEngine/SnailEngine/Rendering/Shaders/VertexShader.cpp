#include "stdafx.h"

#include "VertexShader.h"

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"

namespace Snail
{
VertexShader::VertexShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* inputLayoutDescs, const UINT nbLayout)
{
    ID3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice()->GetD3DDevice();

    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pVSErrorBlob = nullptr;

    DX_COMPILE_SHADER(pVSErrorBlob,
                      D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0",
                          D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &pVSErrorBlob),
                      DXE_FICHIER_PS);

    DX_CALL(device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &InternalShader), DXE_CREATION_VS);

    DX_CALL(device->CreateInputLayout(inputLayoutDescs, nbLayout, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &InputLayout),
            DXE_CREATIONLAYOUT);

    Reflect(pVSBlob);
    pVSBlob->Release();
}

void VertexShader::Bind()
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    context->IASetInputLayout(InputLayout);
    context->VSSetShader(InternalShader, nullptr, 0);
    context->VSSetConstantBuffers(0, static_cast<UINT>(constantBuffers.size()), constantBuffers.data());
}

VertexShader::~VertexShader()
{
    DX_RELEASE(InternalShader);
    DX_RELEASE(InputLayout);
}
}
