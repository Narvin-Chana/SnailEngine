#include "stdafx.h"
#include "Sprite.h"

#include "SpriteVertex.h"
#include "Core/WindowsEngine.h"
#include "Rendering/Shaders/EffectsShader.h"

namespace Snail
{
Sprite::Params::Params()
    : texture(
        WindowsEngine::GetModule<TextureManager>().GetTexture2D(
            TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME
        )
    )
{ }

void Sprite::PrepareDraw() const
{
    UIElement::PrepareDraw();
    drawEffect->BindTexture("Sprite", texture);
}

void Sprite::DrawGeometry() const
{
    UIElement::DrawGeometry();
    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->Draw(6);
}

Sprite::Sprite(const Params& params)
    : UIElement{params}
    , texture{params.texture}
{
}

void Sprite::SetTexture(Texture2D* tex)
{
    texture = tex;
}
Sprite::~Sprite() = default;

void Sprite::InitShaders()
{
    drawEffect = std::make_unique<EffectsShader>(L"SnailEngine/Shaders/UIElements/Sprite.fx", SpriteVertex::layout, SpriteVertex::elementCount);
}
}
