#pragma once
#include "Shader.h"

struct ID3D11GeometryShader;

namespace Snail
{

class GeometryShader : public Shader {
	ID3D11GeometryShader* internalShader = nullptr;
public:
	GeometryShader(const std::wstring& filename);
	void Bind() override;
	~GeometryShader() override;
};

};