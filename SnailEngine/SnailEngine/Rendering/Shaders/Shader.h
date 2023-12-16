#pragma once
#include <map>
#include <vector>
#include <d3dcommon.h>
#include <string>

struct ID3D11Buffer;

namespace Snail
{
class Shader
{
protected:
	std::map<std::string, int> NameToIndex;
	std::vector<ID3D11Buffer*> constantBuffers;

public:
	Shader() = default;
	Shader(Shader&&) = default;
	Shader& operator=(Shader&&) = default;

	// Could be default
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	virtual ~Shader() = default;

	void Reflect(ID3DBlob* blob);
	virtual void SetConstantBuffer(int index, ID3D11Buffer* buffer);
	virtual void SetConstantBuffer(const std::string& buffName, ID3D11Buffer* buffer);
	virtual void Bind() = 0;
};
};
