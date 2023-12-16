#include "stdafx.h"

#include "TimeUI.h"
#include "Text.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
TimeUI::TimeUI(bool _isLap)
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}}}
    , isLap{_isLap}
{
    Text::Params txtParams;
    txtParams.parentHorizontalAnchor = Alignment::START;
    txtParams.parentVerticalAnchor = Alignment::END;
    txtParams.localHorizontalAnchor = Alignment::START;
    txtParams.localVerticalAnchor = Alignment::START;
    txtParams.transform.position.y = isLap ? -100.0f : -50.0f;
    txtParams.transform.position.x = 10;
    txtParams.transform.scale = Vector2{0.7f, 0.7f};
    txtParams.text = "TIMER";
    txtParams.font = WindowsEngine::GetDefaultFont();
    timeText = dynamic_cast<Text*>(AddChild(CreatePtr<Text>(txtParams)));
}

std::string TimeUI::ConvertTime(const float time) const
{
    int minutes = static_cast<int>(time) / 60;
    int remainingSeconds = static_cast<int>(time) % 60;
    float milliseconds = (time - static_cast<int>(time)) * 1000;

    return std::format("{:02d}:{:02d}:{:03d}", minutes, remainingSeconds, static_cast<int>(milliseconds));
}

void TimeUI::Update(const float dt, const float elapsedTime)
{
    UIElement::Update(dt);
    timeText->SetText((isLap ? "Lap : " : "Timer : ") + ConvertTime(elapsedTime));
}
}
