#pragma once
#include "Image.h"
#include "Texture.h"

struct ID3D11Texture2D;

namespace Snail
{

class Texture2D : public Texture
{
    bool generateMips = true;

    void InitResources(const Image& image) override;
	void InitShaderResource(const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT viewFormat);
public:
    Texture2D(const std::wstring& filename);
    Texture2D(const std::string& filename, bool generateMips = true);
    Texture2D(const Image& image, bool generateMips = true);
    Texture2D(int width, int height, const Color& color);
    Texture2D(const D3D11_TEXTURE2D_DESC& texDesc, DXGI_FORMAT viewFormat, bool generateMips = false);
    Texture2D(const D3D11_TEXTURE2D_DESC& texDesc, DXGI_FORMAT viewFormat, const D3D11_SUBRESOURCE_DATA& initialData, bool generateMips = false);
    ~Texture2D() override;
};

}
