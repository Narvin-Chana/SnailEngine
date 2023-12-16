#pragma once

#include "Shader.h"

struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

namespace Snail 
{

class VertexShader : public Shader {
	ID3D11VertexShader* InternalShader = nullptr;
	ID3D11InputLayout* InputLayout = nullptr;
public:
	VertexShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC* inputLayoutDescs, const UINT nbLayout);
	void Bind() override;
	~VertexShader() override;
};

};