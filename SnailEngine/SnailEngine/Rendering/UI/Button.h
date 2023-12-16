#pragma once
#include "UIElement.h"

namespace Snail
{
class Sprite;
class Texture2D;

class Button : public UIElement
{
    bool isHovered = false;
    Sprite* background;
    Texture2D* backgroundTexture;
    Texture2D* hoveredTexture;
    std::function<void()> onClick;
public:
    struct Params : UIElement::Params
    {
        Params();

        std::string text;
        Texture2D* backgroundTexture;
        Texture2D* hoveredTexture;
        std::function<void()> onClick;
    };

    Button(const Params& params = {});
    Button(Button&&) = default;
    Button& operator=(Button&&) = default;

    void Update(float) override;
};
}
