#pragma once

#include <DirectXMath.h>
#include <Windows.h>
#include <sstream>

template<class ...T>
void DebugPrint(T&&... args) {
	std::stringstream ss;
	((ss << args << " "), ...);
	OutputDebugStringA(ss.str().c_str());
}

void DebugPrintVector4(const DirectX::XMFLOAT4& vec);
void DebugPrintVector2(const DirectX::XMFLOAT2& vec);
void DebugPrintVector3(const Vector3& vec);
void DebugPrintMat3x3(const DirectX::XMFLOAT3X3& mat);
void DebugPrintMat4x4(const DirectX::XMFLOAT4X4& mat);