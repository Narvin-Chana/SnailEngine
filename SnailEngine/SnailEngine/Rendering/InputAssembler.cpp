#include "stdafx.h"
#include "InputAssembler.h"
#include "D3D11Device.h"

namespace Snail
{

void InputAssembler::SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY topology)
{
	static const D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();
	renderDevice->GetImmediateContext()->IASetPrimitiveTopology(topology);
}

void InputAssembler::SetVertexBuffer(const D3D11Buffer& buffer, const UINT stride, const UINT offset)
{
	static const D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();
	renderDevice->GetImmediateContext()->IASetVertexBuffers(
		0, 1, &buffer.GetBuffer(), &stride, &offset);
}

void InputAssembler::SetIndexBuffer(const D3D11Buffer& buffer)
{
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	switch (buffer.GetBufferElementWidth())
	{
	case sizeof(uint16_t):
		format = DXGI_FORMAT_R16_UINT;
		break;
	case sizeof(uint32_t):
		format = DXGI_FORMAT_R32_UINT;
		break;
	default:
		break;
	}


	static const D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

	renderDevice->GetImmediateContext()->IASetIndexBuffer(
		buffer.GetBuffer(), format, 0);
}

void InputAssembler::SetInstanceBuffer(const D3D11Buffer& instanceBuffer, const UINT stride, const UINT offset)
{
    static const D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();
    renderDevice->GetImmediateContext()->IASetVertexBuffers(1, 1, &instanceBuffer.GetBuffer(), &stride, &offset);
}
};
