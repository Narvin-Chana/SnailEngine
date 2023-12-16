#include "stdafx.h"
#include "Button.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
Button::Params::Params()
    : backgroundTexture(WindowsEngine::GetModule<TextureManager>().GetTexture2D(TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME))
    , hoveredTexture{backgroundTexture}
    , onClick([]{})
{
}

Button::Button(const Params& params)
    : UIElement{params}
    , backgroundTexture{params.backgroundTexture}
    , hoveredTexture{params.hoveredTexture}
    , onClick{params.onClick}
{
    Sprite::Params bgParams;
    bgParams.size = Vector2{ 1, 1 };
    bgParams.sizeType = SizeType::TYPE_PERCENTAGE;
    bgParams.parentHorizontalAnchor = Alignment::MIDDLE;
    bgParams.parentVerticalAnchor = Alignment::MIDDLE;
    bgParams.localHorizontalAnchor = Alignment::MIDDLE;
    bgParams.localVerticalAnchor = Alignment::MIDDLE;
    bgParams.texture = params.backgroundTexture;
    background = reinterpret_cast<Sprite*>(AddChild(CreatePtr<Sprite>(bgParams)));

    Text::Params txtParams;
    txtParams.parentHorizontalAnchor = Alignment::MIDDLE;
    txtParams.parentVerticalAnchor = Alignment::MIDDLE;
    txtParams.localHorizontalAnchor = Alignment::MIDDLE;
    txtParams.localVerticalAnchor = Alignment::MIDDLE;
    txtParams.text = params.text;
    txtParams.transform.scale = Vector2{.5f, .5f};
    txtParams.font = WindowsEngine::GetDefaultFont();
    AddChild(CreatePtr<Text>(txtParams));
}

void Button::Update(float dt)
{
    UIElement::Update(dt);

    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    static auto& mouse = InputModule::GetInstance().Mouse;

    Vector2 pos = mouse.GetMousePosition();
    pos.y = device->GetResolutionSize().y - pos.y;
    const Quad corners = GetScreenSpaceQuad();


    const bool intersect = corners.Insersect(pos);
    if (isHovered && !intersect)
    {
        isHovered = false;
        background->SetTexture(backgroundTexture);
    }

    if (!isHovered && intersect)
    {
        isHovered = true;
        background->SetTexture(hoveredTexture);
    }

    if (isHovered && mouse.IsButtonPressed(&Mouse::MouseButtonState::leftButton))
        onClick();
}
}
