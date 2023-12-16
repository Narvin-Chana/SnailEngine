#pragma once
#include <d3d11.h>

#include "Core/WindowsEngine.h"

namespace Snail
{

class D3D11Buffer;

struct InputAssembler
{
	static void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	static void SetVertexBuffer(const D3D11Buffer& buffer, UINT stride, UINT offset);
	static void SetIndexBuffer(const D3D11Buffer& buffer);
    static void SetInstanceBuffer(const D3D11Buffer& instanceBuffer, UINT stride, UINT offset);
};

};
