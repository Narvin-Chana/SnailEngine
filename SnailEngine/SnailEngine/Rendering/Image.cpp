#include "stdafx.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>

namespace Snail {

Image::Image(const std::string& filename)
{
    if (!std::filesystem::exists(filename))
        throw FileNotFoundException(filename);

	data = stbi_load(filename.c_str(), &width, &height, &components, REQUIRED_COMPONENTS);
}

Image::Image(const int width, const int height, const Color& color)
	: width(width)
	, height(height)
	, components(REQUIRED_COMPONENTS)
{
	const auto bufferWidth = width * height * REQUIRED_COMPONENTS;
	data = new uint8_t[bufferWidth];

	for (int i = 0; i < bufferWidth; i += REQUIRED_COMPONENTS)
	{
		data[i] = color.x;
		data[i+1] = color.y;
		data[i+2] = color.z;
		data[i+3] = color.w;
	}
}

Image::~Image()
{
	stbi_image_free(data);
}

Color Image::GetPixel(const int x, const int y) const
{
	const uint8_t* rgb = data + ((y * width + x) * REQUIRED_COMPONENTS);
	return Color(rgb[0] / 256.0f, rgb[1] / 256.0f, rgb[2] / 256.0f, rgb[3] / 256.0f);
}

int Image::GetWidth() const
{
	return width;
}

int Image::GetHeight() const
{
	return height;
}

uint8_t* Image::GetData() const
{
	return data;
}

int Image::GetByteWidth() const
{
	return width * REQUIRED_COMPONENTS * sizeof(uint8_t);
}

DirectX::XMINT4 Image::GetPixelInt(const int x, const int y) const
{
    const uint8_t* rgb = data + (y * width + x) * REQUIRED_COMPONENTS;
	return DirectX::XMINT4(rgb[0], rgb[1], rgb[2], rgb[3]);
}

};
