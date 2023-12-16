#pragma once
#include <DirectXMath.h>
#include <string>

namespace Snail {

class Image {
	uint8_t* data;
	static constexpr int REQUIRED_COMPONENTS = 4;
	int width{}, height{}, components{};
public:
	Image(const std::string& filename);
	Image(int width, int height, const Color& color);
	virtual ~Image();

    Color GetPixel(int x, int y) const;
	int GetWidth() const;
	int GetHeight() const;
	uint8_t* GetData() const;
	int GetByteWidth() const;
	DirectX::XMINT4 GetPixelInt(int x, int y) const;
};

}
