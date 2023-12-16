#include "stdafx.h"
#include "Texture2D.h"

#include <d3d11.h>
#include <DDSTextureLoader.h>

#include "Util/Util.h"
#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"

namespace Snail
{
void Texture2D::InitResources(const Image& image)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const UINT mipLevel = static_cast<UINT>(1 + log2(std::max(width, height)));

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Height = height;
    desc.Width = width;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MipLevels = mipLevel;
    desc.ArraySize = 1;

    if (generateMips)
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    DX_CALL(renderDevice->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &rawTexture), DXE_ERROR_CREATING_TEXTURE);

    {
        std::lock_guard lock{DeviceMutex};
        renderDevice->GetImmediateContext()->UpdateSubresource(rawTexture,
                                                               0,
                                                               nullptr,
                                                               image.GetData(),
                                                               image.GetByteWidth(),
                                                               image.GetByteWidth() * image.GetHeight());

        InitShaderResource(desc, desc.Format);
    }
}

void Texture2D::InitShaderResource(const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT viewFormat)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    shaderDesc.Format = viewFormat;

    if (desc.ArraySize == 1)
    {
        shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderDesc.Texture2D.MipLevels = desc.MipLevels;
        shaderDesc.Texture2D.MostDetailedMip = 0;
    }
    else if (desc.ArraySize > 1)
    {
        shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        shaderDesc.Texture2DArray.ArraySize = desc.ArraySize;
        shaderDesc.Texture2DArray.FirstArraySlice = 0;
        shaderDesc.Texture2DArray.MipLevels = desc.MipLevels;
        shaderDesc.Texture2DArray.MostDetailedMip = 0;
    }

    DX_CALL(renderDevice->GetD3DDevice()->CreateShaderResourceView(rawTexture, &shaderDesc, &shaderResourceView),
            DXE_ERROR_CREATING_SHADER_VIEW);

    if (generateMips)
        renderDevice->GetImmediateContext()->GenerateMips(shaderResourceView);
}

Texture2D::Texture2D(const std::wstring& filename)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    if (filename.find(L".dds") != std::wstring::npos)
    {
        DX_CALL(DirectX::CreateDDSTextureFromFile(renderDevice->GetD3DDevice(), filename.c_str(), nullptr, &shaderResourceView),
                DXE_ERROR_CREATING_SHADER_VIEW);
    }
    else
    {
        const Image image(WStringToString(filename));
        Texture2D::InitResources(image);
    }
    Texture::InitSampler();
}

Texture2D::Texture2D(const std::string& filename, bool generateMips)
    : Texture2D(Image(filename), generateMips)
{
#ifdef _PRIVATE_DATA
    D3D11Device::SetDebugName(samplerState, "Sampler-" + filename);
    D3D11Device::SetDebugName(rawTexture, "Texture-" + filename);
    D3D11Device::SetDebugName(shaderResourceView, "SRV-" + filename);
#endif
}

Texture2D::Texture2D(const int width, const int height, const Color& color)
    : Texture2D(Image(width, height, color), true)
{}

Texture2D::Texture2D(const D3D11_TEXTURE2D_DESC& texDesc, const DXGI_FORMAT viewFormat, bool generateMips)
    : generateMips{ generateMips }
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    DX_CALL(renderDevice->GetD3DDevice()->CreateTexture2D(&texDesc, nullptr, &rawTexture), DXE_ERROR_CREATING_TEXTURE);

    InitShaderResource(texDesc, viewFormat);
    Texture2D::InitSampler();
}

Texture2D::Texture2D(const D3D11_TEXTURE2D_DESC& texDesc, const DXGI_FORMAT viewFormat, const D3D11_SUBRESOURCE_DATA& initialData, bool generateMips)
    : generateMips{ generateMips }
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    DX_CALL(renderDevice->GetD3DDevice()->CreateTexture2D(&texDesc, &initialData, &rawTexture), DXE_ERROR_CREATING_TEXTURE);

    InitShaderResource(texDesc, viewFormat);
    Texture2D::InitSampler();
}

Texture2D::Texture2D(const Image& image, bool generateMips)
    : generateMips{generateMips}
{
    Texture2D::InitResources(image);
    Texture::InitSampler();

#ifdef _PRIVATE_DATA
    static int i = 0;
    i++;
    D3D11Device::SetDebugName(samplerState, "Unnamed-Sampler-" + std::to_string(i));
    D3D11Device::SetDebugName(rawTexture, "Unnamed-Texture-" + std::to_string(i));
    D3D11Device::SetDebugName(shaderResourceView, "Unnamed-SRV-" + std::to_string(i));
#endif
}

Texture2D::~Texture2D()
{
    DX_RELEASE(shaderResourceView);
    DX_RELEASE(rawTexture);
}

}
