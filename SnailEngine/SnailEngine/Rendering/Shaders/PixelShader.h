#pragma once
#include "Shader.h"

#include <string>

struct ID3D11PixelShader;

namespace Snail {

class PixelShader : public Shader {
	ID3D11PixelShader* internalShader = nullptr;
public:
	PixelShader(const std::wstring& filename);
	~PixelShader() override;
	void Bind() override;
};

}