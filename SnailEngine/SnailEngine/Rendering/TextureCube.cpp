#include "stdafx.h"
#include "TextureCube.h"

#include <vector>
#include <array>

#include "Core/WindowsEngine.h"
#include "Core/WindowsResource/resource.h"

namespace Snail
{
void TextureCube::InitResources(const Image& image)
{
    static D3D11Device* renderDevice = WindowsEngine::GetInstance().GetRenderDevice();

    int cubeSide = image.GetWidth() / 4;
    if (int cubeSideH = image.GetHeight() / 3; cubeSideH != cubeSide)
        throw DXE_WRONG_CUBEMAP_FORMAT;

    std::vector<Vector4> faces{};
    faces.reserve(cubeSide * cubeSide * 6);

    // following D3D11_TEXTURECUBE_FACE enum
    std::array<D3D11_SUBRESOURCE_DATA, 6> subresourcesData;
    std::array startOffsets{
        DirectX::XMINT2{cubeSide * 2, cubeSide}, // POS X
        DirectX::XMINT2{0, cubeSide}, // NEG X
        DirectX::XMINT2{cubeSide, 0}, // POS Y
        DirectX::XMINT2{cubeSide, cubeSide * 2}, // NEG Y
        DirectX::XMINT2{cubeSide, cubeSide}, // POS Z
        DirectX::XMINT2{cubeSide * 3, cubeSide}, // NEG Z
    };

    for (int i = 0; i < startOffsets.size(); ++i)
    {
        const auto [topX, topY] = startOffsets[i];
        for (int j = 0; j < (cubeSide * cubeSide); ++j)
        {
            const auto row = j / cubeSide;
            const auto xOffset = j % cubeSide;
            faces.push_back(image.GetPixel(topX + xOffset, topY + row));
        }

        subresourcesData[i].pSysMem = faces.data() + (cubeSide * cubeSide * i);
        subresourcesData[i].SysMemPitch = cubeSide * sizeof(Vector4);
        subresourcesData[i].SysMemSlicePitch = 0;
    }

    D3D11_TEXTURE2D_DESC desc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.Height = cubeSide;
    desc.Width = cubeSide;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    DX_CALL(
        renderDevice->GetD3DDevice()->CreateTexture2D(&desc, subresourcesData.data(), &rawTexture),
        DXE_ERROR_CREATING_TEXTURE
    );

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    shaderDesc.Format = desc.Format;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    shaderDesc.TextureCube.MipLevels = 1;
    shaderDesc.TextureCube.MostDetailedMip = 0;
    DX_CALL(
        renderDevice->GetD3DDevice()->CreateShaderResourceView(rawTexture, &shaderDesc, &shaderResourceView),
        DXE_ERROR_CREATING_SHADER_VIEW
    );
}

TextureCube::TextureCube(const std::wstring& filename)
    : TextureCube(WStringToString(filename)) {}

TextureCube::TextureCube(const std::string& filename)
    : TextureCube(Image(filename)) {}

TextureCube::TextureCube(const Image& image)
{
    TextureCube::InitResources(image);
    Texture::InitSampler();
}

TextureCube::~TextureCube()
{
    DX_RELEASE(shaderResourceView);
    DX_RELEASE(rawTexture);
}
}
