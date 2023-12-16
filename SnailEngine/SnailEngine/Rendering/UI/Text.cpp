#include "stdafx.h"
#include "Text.h"

#include "TextVertex.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
void Text::PrepareDraw() const
{
    UIElement::PrepareDraw();
    drawEffect->BindTexture("Text", font->GetFontTexture());
    drawEffect->SetConstantBuffer("Chars", characterBuffer.GetBuffer());
    const Matrix transformMat = GetTransformMatrix().Transpose();
    textInfoBuffer.UpdateData(transformMat);
    drawEffect->SetConstantBuffer("TextInfo", textInfoBuffer.GetBuffer());
}

void Text::DrawGeometry() const
{
    UIElement::DrawGeometry();
    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->DrawInstanced(6, static_cast<UINT>(text.size()), 0, 0);
}

Text::Text(const Params& params)
    : UIElement{params}
    , text{params.text}
    , font{params.font}
    , characterBuffer{D3D11Buffer::CreateConstantBuffer<CharacterBuffer[MAX_CHARACTER_COUNT]>()}
    , textInfoBuffer{D3D11Buffer::CreateConstantBuffer<Matrix>()}
{
    std::lock_guard _{ DeviceMutex };
    SetText(text);
}

void Text::SetText(const std::string& newText)
{
    text = newText;
    originalSize = Vector2{0, 0};
    charactersBufferData.clear();

    for (const char c : newText)
    {
        const auto& letter = font->GetCharacter(c);
        if (!letter)
            continue;

        Vector2 pos{static_cast<float>(letter->x), static_cast<float>(letter->y)};
        Vector2 charSize{static_cast<float>(letter->width), static_cast<float>(letter->height)};
        const Vector2 offset{static_cast<float>(letter->originX), static_cast<float>(letter->originY)};

        originalSize.x -= offset.x;

        Transform2D localTransform;
        localTransform.position = Vector2{originalSize.x, offset.y - charSize.y};
        originalSize.y = std::max(originalSize.y, charSize.y);
        originalSize.x += charSize.x;
        charactersBufferData.emplace_back(localTransform.GetTransformMatrix().Transpose(), pos, charSize);
    }

    size = originalSize;
    for (auto& ch : charactersBufferData)
    {
        Vector2 offset{};
        ApplyLocalOffset(offset, localVerticalAnchor, localHorizontalAnchor);
        ch.anchorMatrix *= Matrix::CreateTranslation(Vector3{offset.x, offset.y, 0}).Transpose();
    }

    characterBuffer.UpdateData(charactersBufferData);
}

void Text::InitShaders()
{
    drawEffect = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/UIElements/Text.fx", TextVertex::layout, TextVertex::elementCount);
}
}
