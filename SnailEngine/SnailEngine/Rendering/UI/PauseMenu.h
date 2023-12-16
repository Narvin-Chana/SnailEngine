#pragma once
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class PauseMenu : public UIElement
{
    Texture2D* buttonTextureBG;
    Texture2D* buttonTextureHover;
    Texture2D* titleTex;
    Texture2D* background;

    Texture2D* banana;
public:
    PauseMenu();
    PauseMenu(PauseMenu&&) = default;
    PauseMenu& operator=(PauseMenu&&) = default;
    void Update(float) override;
};
}
