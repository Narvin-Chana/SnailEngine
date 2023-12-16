#pragma once
#include <string>
#include <array>

#include "Font.h"
#include "Sprite.h"
#include "Util/Util.h"

namespace Snail
{
class Text : public UIElement
{
    std::string text;
    const Font* font;

    struct DX_ALIGN CharacterBuffer
    {
        Matrix anchorMatrix;
        Vector2 charPosition;
        Vector2 charSize;
    };

    struct TextInfo
    {
        int verticalAlignment;
        int horizontalAlignment;
    };

    static constexpr auto MAX_CHARACTER_COUNT = 128;
    FixedVector<CharacterBuffer, MAX_CHARACTER_COUNT> charactersBufferData;
    D3D11Buffer characterBuffer;
    mutable D3D11Buffer textInfoBuffer;

    void PrepareDraw() const override;
    void DrawGeometry() const override;

public:
    struct Params : UIElement::Params
    {
        std::string text;
        const Font* font;
    };

    Text(const Params& params = {});
    Text(Text&&) = default;
    Text& operator=(Text&&) = default;
    void SetText(const std::string& newText);
    void InitShaders() override;
};
}
