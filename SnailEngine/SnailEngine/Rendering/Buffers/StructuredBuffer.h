#pragma once
#include "Util/Util.h"

namespace Snail
{

struct StructuredBuffer
{
    UINT numElements = 0;
    ID3D11Buffer* internalBuffer = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11UnorderedAccessView* uav = nullptr;

    StructuredBuffer() = default;
    template <class T>
    StructuredBuffer(ID3D11Device* pd3dDevice, const UINT numberOfElements, const T* initialData = nullptr)
        : numElements{numberOfElements}
    {
        // Create SB
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = numElements * sizeof(T);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(T);

        D3D11_SUBRESOURCE_DATA bufferInitData = {};
        bufferInitData.pSysMem = initialData;
        DX_CALL(pd3dDevice->CreateBuffer(&bufferDesc, initialData ? &bufferInitData : nullptr, &internalBuffer), "Error creating structured buffer.");

        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.ElementWidth = numElements;
        DX_CALL(pd3dDevice->CreateShaderResourceView(internalBuffer, &srvDesc, &srv), "Error creating SRV of structured buffer.");

        // Create UAV
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = numElements;
        DX_CALL(pd3dDevice->CreateUnorderedAccessView(internalBuffer, &uavDesc, &uav), "Error creating UAV of structured buffer.");
    }

    StructuredBuffer(const StructuredBuffer&) = delete;
    StructuredBuffer& operator=(const StructuredBuffer&) = delete;

    StructuredBuffer(StructuredBuffer&& buffer) noexcept;
    StructuredBuffer& operator=(StructuredBuffer&& buffer) noexcept;

    ~StructuredBuffer();

    void Swap(StructuredBuffer& buf);
};

}
