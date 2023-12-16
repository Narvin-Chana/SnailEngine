#pragma once
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class MainMenu : public UIElement
{
    Texture2D* buttonTextureBG;
    Texture2D* buttonTextureHover;
    Texture2D* titleTex;
    Texture2D* background;
    Texture2D* logo;

    Texture2D* banana;
public:
    MainMenu();
    MainMenu(MainMenu&&) = default;
    MainMenu& operator=(MainMenu&&) = default;
    void Update(float) override;
};
}
