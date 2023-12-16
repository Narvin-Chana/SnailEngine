#include "stdafx.h"
#include "Font.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
Font::Font(const FontData& font)
    : innerFont{font}
    , texture{ WindowsEngine::GetModule<TextureManager>().GetTexture2D(font.name, true) }
{
}

Texture2D* Font::GetFontTexture() const
{
    return texture;
}

std::optional<Character> Font::GetCharacter(char c) const
{
    const auto begin = innerFont.characters;
    const auto end = innerFont.characters + innerFont.characterCount;
    auto it = std::find_if(begin, end, [&](const Character& car) {
        return car.codePoint == c;
    });

    if (it == end)
    {
        LOGF("Character '{}' doesnt exist in font \"{}\"", c, innerFont.name);
        return {};
    }

    return *it;
}
}
