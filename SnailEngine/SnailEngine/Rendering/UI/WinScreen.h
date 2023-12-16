#pragma once
#include "Text.h"
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class WinScreen : public UIElement
{
    std::unique_ptr<Texture2D> buttonTextureBG;
    std::unique_ptr<Texture2D> buttonTextureHover;
    std::unique_ptr<Texture2D> titleTex;
    std::unique_ptr<Texture2D> background;

    Text* timeText;
    std::vector<Text*> lapsTexts;

    std::string ConvertTime(const float time) const;

public:
    WinScreen();
    WinScreen(WinScreen&&) = default;
    WinScreen& operator=(WinScreen&&) = default;

    void SetTimesText(float time, std::vector<float> lapTimes);
    void SetLapCount(int lapCount);
};
}
