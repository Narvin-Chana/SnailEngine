#include "stdafx.h"
#include "EffectsShader.h"

#include <d3dx11effect.h>

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"

#include "Rendering/Texture.h"

namespace Snail
{
void EffectsShader::BindViewAndSampler(const std::string& textureName, ID3D11ShaderResourceView* view, ID3D11SamplerState* state) const
{
    ID3DX11EffectShaderResourceVariable* variableTexture = effect->GetVariableByName(textureName.c_str())->AsShaderResource();
    variableTexture->SetResource(view);
    // Le sampler state
    ID3DX11EffectSamplerVariable* variableSampler = effect->GetVariableByName((textureName + "Sampler").c_str())->AsSampler();
    variableSampler->SetSampler(0, state);
}

void EffectsShader::BindViewAndSampler(const int textureIndex, ID3D11ShaderResourceView* view, ID3D11SamplerState* state) const
{
    ID3DX11EffectShaderResourceVariable* variableTexture = effect->GetVariableByIndex(textureIndex)->AsShaderResource();
    variableTexture->SetResource(view);
    // Le sampler state
    ID3DX11EffectSamplerVariable* variableSampler = effect->GetVariableByIndex(textureIndex)->AsSampler();
    variableSampler->SetSampler(0, state);
}

void EffectsShader::BindView(const int textureIndex, ID3D11ShaderResourceView* view) const
{
    ID3DX11EffectShaderResourceVariable* variableTexture = effect->GetVariableByIndex(textureIndex)->AsShaderResource();
    variableTexture->SetResource(view);
}

void EffectsShader::BindView(const std::string& textureName, ID3D11ShaderResourceView* view) const
{
    ID3DX11EffectShaderResourceVariable* variableTexture = effect->GetVariableByName(textureName.c_str())->AsShaderResource();
    variableTexture->SetResource(view);
}

void EffectsShader::ReloadShader()
{
    const auto device = WindowsEngine::GetInstance().GetRenderDevice()->GetD3DDevice();

    ID3DBlob* pFXBlob = nullptr;
    ID3DBlob* pFXErrorBlob = nullptr;

    std::vector<D3D_SHADER_MACRO> macros;
    std::ranges::transform(defines, std::back_inserter(macros), [](const std::string& define)-> D3D_SHADER_MACRO {return { define.c_str(), nullptr }; });

    // this acts as a sentinel value to indicate the end of the macros
    macros.emplace_back(nullptr, nullptr);

    auto result = D3DCompileFromFile(shaderName.c_str(),
        macros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        nullptr,
        "fx_5_0",
        0,
        0,
        &pFXBlob,
        &pFXErrorBlob);

    DX_COMPILE_SHADER(pFXErrorBlob, result, DXE_ERREURCREATION_FX);

    DX_RELEASE(effect);
    D3DX11CreateEffectFromMemory(pFXBlob->GetBufferPointer(), pFXBlob->GetBufferSize(), 0, device, &effect);

    DX_RELEASE(pFXErrorBlob);
    DX_RELEASE(pFXBlob);

    effectTechnique = effect->GetTechniqueByIndex(0);
    effectPass = effectTechnique->GetPassByIndex(0);
    
    D3DX11_PASS_SHADER_DESC effectVSDesc;
    effectPass->GetVertexShaderDesc(&effectVSDesc);

    D3DX11_EFFECT_SHADER_DESC effectVSDesc2;
    effectVSDesc.pShaderVariable->GetShaderDesc(effectVSDesc.ShaderIndex, &effectVSDesc2);

    DX_RELEASE(inputLayout);
    DX_CALL(device->CreateInputLayout(inputLayoutDesc, inputLayoutCount, effectVSDesc2.pBytecode, effectVSDesc2.BytecodeLength, &inputLayout),
        DXE_CREATIONLAYOUT);
}

EffectsShader::EffectsShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* inputLayoutDescs, const UINT nbLayout, const std::unordered_set<std::string>& defines)
    : defines{defines}
    , shaderName{filename}
    , inputLayoutDesc{inputLayoutDescs}
    , inputLayoutCount{nbLayout}
{
    ReloadShader();
}

EffectsShader::~EffectsShader()
{
    DX_RELEASE(effect);
    DX_RELEASE(internalShader);
    DX_RELEASE(inputLayout);
}

void EffectsShader::Bind()
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    context->IASetInputLayout(inputLayout);

    for (int i = 0; i < constantBuffers.size(); ++i) { effect->GetConstantBufferByIndex(i)->SetConstantBuffer(constantBuffers[i]); }
    effectPass->Apply(0, context);
}

void EffectsShader::AddDefine(const std::string& define)
{
    defines.insert(define);
    ReloadShader();
}

bool EffectsShader::HasDefine(const std::string& define) const
{
    return defines.contains(define);
}

void EffectsShader::RemoveDefine(const std::string& define)
{
    defines.erase(define);
    ReloadShader();
}

void EffectsShader::BindTexture(const std::string& textureName, const Texture* tex)
{
    BindViewAndSampler(textureName, tex->GetShaderResourceView(), tex->GetSamplerState());
}

void EffectsShader::BindTexture(const int index, const Texture* tex)
{
    BindViewAndSampler(index, tex->GetShaderResourceView(), tex->GetSamplerState());
}

void EffectsShader::BindShaderResourceView(const int index, ID3D11ShaderResourceView* view)
{
    BindView(index, view);
}

void EffectsShader::BindShaderResourceView(const std::string& resourceName, ID3D11ShaderResourceView* view)
{
    BindView(resourceName, view);
}

void EffectsShader::BindShaderResourceViewAndSampler(const int index, ID3D11ShaderResourceView* view, ID3D11SamplerState* samplerState)
{
    BindViewAndSampler(index, view, samplerState);
}

void EffectsShader::BindShaderResourceViewAndSampler(const std::string& resourceName, ID3D11ShaderResourceView* view, ID3D11SamplerState* samplerState)
{
    BindViewAndSampler(resourceName, view, samplerState);
}

void EffectsShader::UnbindResource(const std::string& resourceName)
{
    ID3D11DeviceContext* context = WindowsEngine::GetInstance().GetRenderDevice()->GetImmediateContext();
    ID3DX11EffectShaderResourceVariable* variableTexture = effect->GetVariableByName(resourceName.c_str())->AsShaderResource();
    variableTexture->SetResource(nullptr);
    effectTechnique->GetPassByIndex(0)->Apply(0, context);
}

void EffectsShader::SetConstantBuffer(const int index, ID3D11Buffer* buffer)
{
    ID3DX11EffectConstantBuffer* constBuffer = effect->GetConstantBufferByIndex(index);
    constBuffer->SetConstantBuffer(buffer);
}

void EffectsShader::SetConstantBuffer(const std::string& buffName, ID3D11Buffer* buffer)
{
    ID3DX11EffectConstantBuffer* constBuffer = effect->GetConstantBufferByName(buffName.c_str());
    constBuffer->SetConstantBuffer(buffer);
}

}
