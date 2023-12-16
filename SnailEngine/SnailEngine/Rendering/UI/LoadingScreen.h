#pragma once
#include "Sprite.h"
#include "Text.h"
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class LoadingScreen : public UIElement
{
    Sprite* loading;
    std::unique_ptr<Font> font;
    Texture2D* backgroundTex;
    Texture2D* progressTex;
public:
    struct Params : UIElement::Params
    {
        Params();

        Texture2D* loadingSprite;
        Texture2D* backgroundSprite;
    };

    LoadingScreen(const Params& params = {});
    void Update(float dt) override;
    void PrepareDraw() const override;
};
}
