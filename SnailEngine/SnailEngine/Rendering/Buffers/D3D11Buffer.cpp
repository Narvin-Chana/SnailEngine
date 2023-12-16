#include "stdafx.h"

#include "D3D11Buffer.h"

#include "Util/Util.h"
#include "Core/WindowsResource/resource.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
void D3D11Buffer::Swap(D3D11Buffer& buffer)
{
    std::swap(internalBuffer, buffer.internalBuffer);
    std::swap(bindFlags, buffer.bindFlags);
}

D3D11Buffer::D3D11Buffer(D3D11Buffer&& buffer) noexcept
    : internalBuffer{std::exchange(buffer.internalBuffer, nullptr)}
    , bindFlags{std::exchange(buffer.bindFlags, 0)}
{}

D3D11Buffer& D3D11Buffer::operator=(D3D11Buffer&& buffer) noexcept
{
    D3D11Buffer{std::move(buffer)}.Swap(*this);
    return *this;
}

D3D11Buffer::D3D11Buffer(const UINT bindFlags)
    : bindFlags{bindFlags}
{
    if (bindFlags & D3D11_BIND_CONSTANT_BUFFER)
        LOG(Logger::WARN, "Constant buffer created without initial size. Use CreateConstantBuffer() instead.");
}

D3D11Buffer::D3D11Buffer(const UINT bindFlags, const unsigned int size, const void* initialData, const UINT miscFlags)
    : bindFlags{bindFlags}
    , miscFlags{miscFlags}
{
    ResizeBuffer(size, initialData);
}

D3D11Buffer::~D3D11Buffer()
{
    DX_RELEASE(internalBuffer);
}

ID3D11Buffer*& D3D11Buffer::GetBuffer()
{
    return internalBuffer;
}

ID3D11Buffer* const& D3D11Buffer::GetBuffer() const
{
    return internalBuffer;
}

int D3D11Buffer::GetBufferElementWidth() const
{
    return elementByteWidth;
}

void D3D11Buffer::SetBufferElementWidth(const int byteWidth)
{
    elementByteWidth = byteWidth;
}

void D3D11Buffer::UpdateData(const void* val, const unsigned int size)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    if (size == 0)
        return;

    if (internalBuffer == nullptr)
    {
        ResizeBuffer(size, val);
        return;
    }

    D3D11_BUFFER_DESC desc;
    internalBuffer->GetDesc(&desc);

    // If the buffer will always be resized to data and never shrunk. This can be a bit wasteful but it seems to be the easiest way to support variable sized buffer
    if ((bindFlags & D3D11_BIND_CONSTANT_BUFFER) == 0 && desc.ByteWidth < RaiseToNextMultipleOf(size, 16))
        ResizeBuffer(size, val);
    else
        renderDevice->GetImmediateContext()->UpdateSubresource(internalBuffer, 0, nullptr, val, 0, 0);
}

void D3D11Buffer::ResizeBuffer(const unsigned int size, const void* initialData)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    DX_RELEASE(internalBuffer);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = RaiseToNextMultipleOf(size, 16);
    bd.BindFlags = bindFlags;
    bd.MiscFlags = miscFlags;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = initialData;
    DX_CALL(renderDevice->GetD3DDevice()->CreateBuffer( &bd, initialData ? &initData : nullptr, &internalBuffer), DXE_CREATIONVERTEXBUFFER);
}

#ifdef _PRIVATE_DATA
void D3D11Buffer::SetDebugName(const std::string& str) const
{
    D3D11Device::SetDebugName(internalBuffer, str);
}
#endif
}
