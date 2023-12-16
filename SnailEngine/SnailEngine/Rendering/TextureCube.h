#pragma once

#include "Image.h"
#include "Texture.h"

namespace Snail
{
class TextureCube : public Texture
{
    void InitResources(const Image& image) override;

public:
    TextureCube(const std::wstring& filename);
    TextureCube(const std::string& filename);
    TextureCube(const Image& image);
    ~TextureCube() override;
};
}
