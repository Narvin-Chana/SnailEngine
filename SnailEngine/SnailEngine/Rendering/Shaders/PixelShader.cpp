#include "stdafx.h"

#include "PixelShader.h"

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/Resource.h"

namespace Snail
{
PixelShader::PixelShader(const std::wstring& filename)
{
    ID3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice()->GetD3DDevice();
    // Compilation et chargement du pixel shader
    ID3DBlob* pPSBlob = nullptr;
    ID3DBlob* pPSErrorBlob = nullptr;

    DX_COMPILE_SHADER(pPSErrorBlob,
                      D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0",
                          D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob, &pPSErrorBlob),
                      DXE_FICHIER_PS);

    DX_CALL(device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &internalShader), DXE_CREATION_PS);

    

    Reflect(pPSBlob);
    pPSBlob->Release();
}

void PixelShader::Bind()
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    context->PSSetShader(internalShader, nullptr, 0);
    context->PSSetConstantBuffers(0, static_cast<UINT>(constantBuffers.size()), constantBuffers.data());
}

PixelShader::~PixelShader()
{
    DX_RELEASE(internalShader);
}
}
