#include "stdafx.h"
#include "Util/DebugUtil.h"
#include "Core/Engine.h"

void DebugPrintVector4(const DirectX::XMFLOAT4& vec) {

	std::stringstream ss;
	ss << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]";
	OutputDebugStringA(ss.str().c_str());
}

void DebugPrintVector2(const DirectX::XMFLOAT2& vec) {

	std::stringstream ss;
	ss << "[" << vec.x << ", " << vec.y << "]";
	OutputDebugStringA(ss.str().c_str());
}

void DebugPrintVector3(const Vector3& vec) {
	std::stringstream ss;
	ss << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
	OutputDebugStringA(ss.str().c_str());
}

void DebugPrintMat3x3(const DirectX::XMFLOAT3X3& mat) {
	std::stringstream ss;

	ss << "[\n";
	ss << "[" << mat._11 << ", " << mat._12 << ", " << mat._13 << "]\n";
	ss << "[" << mat._21 << ", " << mat._22 << ", " << mat._23 << "]\n";
	ss << "[" << mat._31 << ", " << mat._32 << ", " << mat._33 << "]\n";
	ss << "]";

	OutputDebugStringA(ss.str().c_str());
}

void DebugPrintMat4x4(const DirectX::XMFLOAT4X4& mat) {
	std::stringstream ss;

	ss << "[\n";
	ss << "[" << mat._11 << ", " << mat._12 << ", " << mat._13 << ", " << mat._14 << "]\n";
	ss << "[" << mat._21 << ", " << mat._22 << ", " << mat._23 << ", " << mat._24 << "]\n";
	ss << "[" << mat._31 << ", " << mat._32 << ", " << mat._33 << ", " << mat._34 << "]\n";
	ss << "[" << mat._41 << ", " << mat._42 << ", " << mat._43 << ", " << mat._44 << "]\n";
	ss << "]\n";

	OutputDebugStringA(ss.str().c_str());
}