#include "stdafx.h"
#include "Shader.h"

#include <d3d11shader.h>
#include <d3d11.h>

namespace Snail
{

void Shader::Reflect(ID3DBlob* blob)
{
	ID3D11ShaderReflection* pReflector = nullptr;
	D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection,
	           reinterpret_cast<void**>(&pReflector));

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	constantBuffers.resize(shaderDesc.ConstantBuffers);
	for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		const auto buf = pReflector->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC dsc;
		buf->GetDesc(&dsc);

		NameToIndex[dsc.Name] = i;
	}
}

void Shader::SetConstantBuffer(const int index, ID3D11Buffer* buffer)
{
	constantBuffers[index] = buffer;
}

void Shader::SetConstantBuffer(const std::string& buffName, ID3D11Buffer* buffer)
{
	if (!NameToIndex.contains(buffName))
		return;

	SetConstantBuffer(NameToIndex[buffName], buffer);
}
}
