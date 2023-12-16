#pragma once
#include "Text.h"
#include "UIElement.h"

namespace Snail
{
class Texture2D;

class TimeUI : public UIElement
{
    Text* timeText;
    bool isLap = false;
    std::string ConvertTime(const float time) const;

public:
    TimeUI(bool _isLap = false);
    TimeUI(TimeUI&&) = default;
    TimeUI& operator=(TimeUI&&) = default;
    //float is not dt but time since start for simplicity
    void Update(const float dt, const float time);
};
}
