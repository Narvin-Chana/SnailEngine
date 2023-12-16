#include "stdafx.h"

#include "StructuredBuffer.h"

namespace Snail
{

StructuredBuffer::StructuredBuffer(StructuredBuffer&& buffer) noexcept
    : internalBuffer{std::exchange(buffer.internalBuffer, nullptr)}
    , srv{std::exchange(buffer.srv, nullptr)}
    , uav{std::exchange(buffer.uav, nullptr)} {}

StructuredBuffer& StructuredBuffer::operator=(StructuredBuffer&& buffer) noexcept
{
    StructuredBuffer{std::move(buffer)}.Swap(*this);
    return *this;
}

void StructuredBuffer::Swap(StructuredBuffer& buf)
{
    std::swap(numElements, buf.numElements);
    std::swap(internalBuffer, buf.internalBuffer);
    std::swap(srv, buf.srv);
    std::swap(uav, buf.uav);
}

StructuredBuffer::~StructuredBuffer()
{
    DX_RELEASE(uav);
    DX_RELEASE(srv);
    DX_RELEASE(internalBuffer);
}

}
