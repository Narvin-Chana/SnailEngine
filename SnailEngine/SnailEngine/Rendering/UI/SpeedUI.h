#pragma once
#include "Text.h"
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class SpeedUI : public UIElement
{
    Text* speedText;
    float currentSpeed = 0.0f;
    float elapsedTime = 0.0f;
    std::string ConvertSpeed(const float speed) const;

public:
    SpeedUI();
    SpeedUI(SpeedUI&&) = default;
    SpeedUI& operator=(SpeedUI&&) = default;
    
    void Update(const float, const float speed);
};
}
