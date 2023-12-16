#pragma once
#include <unordered_set>

#include "Shader.h"

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;
struct ID3DX11EffectPass;

namespace Snail {

class Texture;

class EffectsShader : public Shader {
	ID3DX11Effect* effect = nullptr;
	ID3DX11EffectTechnique* effectTechnique = nullptr;
	ID3DX11EffectPass* effectPass = nullptr;
	ID3D11VertexShader* internalShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

    std::unordered_set<std::string> defines;
    std::wstring shaderName;
    const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc;
    UINT inputLayoutCount;

	void BindViewAndSampler(const std::string& textureName, ID3D11ShaderResourceView* view, ID3D11SamplerState* state) const;
	void BindViewAndSampler(int textureIndex, ID3D11ShaderResourceView* view, ID3D11SamplerState* state) const;
	void BindView(int textureIndex, ID3D11ShaderResourceView* view) const;
    void BindView(const std::string& textureName, ID3D11ShaderResourceView* view) const;

public:
	EffectsShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* inputLayoutDescs, UINT nbLayout, const std::unordered_set<std::string>& defines = {});
	~EffectsShader() override;

    void ReloadShader();
	void Bind() override;

    void AddDefine(const std::string& define);
    bool HasDefine(const std::string& define) const;
    void RemoveDefine(const std::string& define);

	void BindTexture(const std::string& textureName, const Texture* tex);
	void BindTexture(int index, const Texture* tex);
	void BindShaderResourceView(int index, ID3D11ShaderResourceView* view);
    void BindShaderResourceView(const std::string& resourceName, ID3D11ShaderResourceView* view);
    void BindShaderResourceViewAndSampler(int index, ID3D11ShaderResourceView* view, ID3D11SamplerState* samplerState);
    void BindShaderResourceViewAndSampler(const std::string& resourceName, ID3D11ShaderResourceView* view, ID3D11SamplerState* samplerState);

    void UnbindResource(const std::string& resourceName);

    void SetConstantBuffer(int index, ID3D11Buffer* buffer) override;
	void SetConstantBuffer(const std::string& buffName, ID3D11Buffer* buffer) override;
};

}
