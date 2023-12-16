#pragma once
#include <optional>

#include "Rendering/Texture2D.h"
#include "Fonts/Arial.h"
static inline constexpr FontData& DEFAULT_FONT = FONT_ARIAL;

namespace Snail
{
class Font
{
    const FontData& innerFont;
    Texture2D* texture;
public:
    Font(const FontData& font = DEFAULT_FONT);
    Texture2D* GetFontTexture() const;
    std::optional<Character> GetCharacter(char c) const;
};
}
