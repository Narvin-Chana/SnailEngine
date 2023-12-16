#include "stdafx.h"
#include "SceneTransition.h"

#include "InfiniteSprite.h"
#include "Sprite.h"
#include "Core/WindowsEngine.h"

namespace Snail
{

SceneTransition::SceneTransition()
    : UIElement{Params{
        .size{200, 200},
        .localVerticalAnchor = Alignment::MIDDLE,
        .localHorizontalAnchor = Alignment::MIDDLE,
        .parentVerticalAnchor = Alignment::MIDDLE,
        .parentHorizontalAnchor = Alignment::MIDDLE,
        .zOrder = 20
    }}
    , anim{Animation::Params{
        .animationLength = 2,
        .shouldStartPaused = false
    }}
{
    static auto& tm = WindowsEngine::GetModule<TextureManager>();

    static constexpr const char* texName = "Resources/snail_stencil.png";
    const auto texture = tm.GetTexture2D(texName, true);

    InfiniteSprite::Params bgParams{};
    bgParams.texture = texture;
    bgParams.size = Vector2{1, 1};
    bgParams.sizeType = SizeType::TYPE_PERCENTAGE;
    bgParams.parentVerticalAnchor = Alignment::START;
    bgParams.parentHorizontalAnchor = Alignment::START;
    bgParams.localVerticalAnchor = Alignment::START;
    bgParams.localHorizontalAnchor = Alignment::START;
    stencil = reinterpret_cast<InfiniteSprite*>(AddChild(CreatePtr<InfiniteSprite>(bgParams)));
}

void SceneTransition::Update(const float dt)
{
    UIElement::Update(dt);
    anim.Update(dt, this);
}

void SceneTransition::Animate(float t)
{
    transform.rotation = t * DirectX::XM_2PI * 2.0f;
    transform.scale = Vector2::Lerp(initialScale, finalScale, log(pow(t, 6.0f) + 1.0f) / log(2.0f));
}

}
