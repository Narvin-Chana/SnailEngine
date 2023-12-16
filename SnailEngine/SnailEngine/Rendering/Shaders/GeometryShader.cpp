#include "stdafx.h"

#include "GeometryShader.h"

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"

namespace Snail
{
GeometryShader::GeometryShader(const std::wstring& filename)
{
    ID3D11Device* device = WindowsEngine::GetInstance().GetRenderDevice()->GetD3DDevice();
    // Compilation et chargement du vertex shader
    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pVSErrorBlob = nullptr;

    DX_COMPILE_SHADER(pVSErrorBlob,
                      D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_5_0",
                          D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &pVSErrorBlob),
                      DXE_FICHIER_PS);

    DX_CALL(device->CreateGeometryShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &internalShader),
            DXE_CREATION_VS);

    Reflect(pVSBlob);
    pVSBlob->Release();
}

void GeometryShader::Bind()
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    context->GSSetShader(internalShader, nullptr, 0);
    if (!constantBuffers.empty())
        context->GSSetConstantBuffers(0, static_cast<UINT>(constantBuffers.size()), constantBuffers.data());
}

GeometryShader::~GeometryShader()
{
    DX_RELEASE(internalShader);
}
}
