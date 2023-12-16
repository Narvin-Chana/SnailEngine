#include "stdafx.h"
#include "LoadingScreen.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
LoadingScreen::Params::Params()
{
    static auto& tm = WindowsEngine::GetModule<TextureManager>();
    backgroundSprite = tm.GetTexture2D("Resources/loading_background.png", true);
    loadingSprite = tm.GetTexture2D("Resources/loading_material.png", true);
    zOrder = 0;
}

LoadingScreen::LoadingScreen(const Params& params)
    : UIElement{UIElement::Params{
        .sizeType = SizeType::TYPE_PERCENTAGE,
        .size = Vector2(1,1),
    }}
{
    Sprite::Params bgParams{};
    bgParams.texture = params.backgroundSprite;
    bgParams.size = Vector2{1, 1};
    bgParams.sizeType = SizeType::TYPE_PERCENTAGE;
    bgParams.parentVerticalAnchor = Alignment::START;
    bgParams.parentHorizontalAnchor = Alignment::START;
    bgParams.localVerticalAnchor = Alignment::START;
    bgParams.localHorizontalAnchor = Alignment::START;
    auto bg = AddChild(CreatePtr<Sprite>(bgParams));

    Sprite::Params loadingParams{};
    loadingParams.texture = params.loadingSprite;
    loadingParams.transform.position = Vector2{ -100, 100 };
    loadingParams.size = Vector2{ 100, 100 };
    loadingParams.parentVerticalAnchor = Alignment::START;
    loadingParams.parentHorizontalAnchor = Alignment::END;
    loadingParams.localVerticalAnchor = Alignment::MIDDLE;
    loadingParams.localHorizontalAnchor = Alignment::MIDDLE;
    // fix rotation of loading bar
    loading = reinterpret_cast<Sprite*>(bg->AddChild(CreatePtr<Sprite>(loadingParams)));
}

void LoadingScreen::Update(const float dt)
{
    UIElement::Update(dt);
    loading->Rotate(std::max(dt, 0.01f));
}

void LoadingScreen::PrepareDraw() const
{
    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->SetAlpha(true);
    UIElement::PrepareDraw();
}
}
