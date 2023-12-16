#pragma once
#include "Util/BufferUtil.h"

struct ID3D11Buffer;

namespace Snail
{
class D3D11Buffer
{
    ID3D11Buffer* internalBuffer = nullptr;
    UINT bindFlags{};
    UINT miscFlags{};
    int elementByteWidth = sizeof(uint32_t);

    void Swap(D3D11Buffer& buffer);

public:
	D3D11Buffer(const D3D11Buffer&) = delete;
    D3D11Buffer& operator=(const D3D11Buffer&) = delete;
	D3D11Buffer(D3D11Buffer&& buffer) noexcept;
	D3D11Buffer& operator=(D3D11Buffer&& buffer) noexcept;

    D3D11Buffer(UINT bindFlags = {});
    D3D11Buffer(UINT bindFlags, unsigned int size, const void* initialData = nullptr, UINT miscFlags = 0);

    template <class T>
    D3D11Buffer(UINT bindFlags, const T& obj, UINT miscFlags = 0);
    ~D3D11Buffer();

    void SetBufferElementWidth(int byteWidth);
    int GetBufferElementWidth() const;

    void UpdateData(const void* val, unsigned int size);

    template <class T>
    void UpdateData(const T& val);

    void ResizeBuffer(unsigned int size, const void* initialData = nullptr);

#ifdef _PRIVATE_DATA
    void SetDebugName(const std::string& str) const;
#endif

    ID3D11Buffer*& GetBuffer();
    ID3D11Buffer* const& GetBuffer() const;

    template<class T>
    static D3D11Buffer CreateConstantBuffer();
};

template <class T>
D3D11Buffer::D3D11Buffer(const UINT bindFlags, const T& obj, const UINT miscFlags)
    : D3D11Buffer(bindFlags, ::GetBufferSize(obj), ::GetBuffer(obj), miscFlags)
{}

template <class T>
void D3D11Buffer::UpdateData(const T& val)
{
    UpdateData(::GetBuffer(val), ::GetBufferSize(val));
}

template <class T>
D3D11Buffer D3D11Buffer::CreateConstantBuffer()
{
    return D3D11Buffer{D3D11_BIND_CONSTANT_BUFFER, static_cast<unsigned int>(sizeof(T))};
}
}
