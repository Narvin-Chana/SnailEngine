#pragma once
#include "Text.h"
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class LapUI : public UIElement
{
    Text* lapText;

public:
    int maxLaps;
    LapUI(int maxlap);
    LapUI(LapUI&&) = default;
    LapUI& operator=(LapUI&&) = default;
    
    void Update(const float, const int);
    void SetLapCount(int lapCount);
};
}
