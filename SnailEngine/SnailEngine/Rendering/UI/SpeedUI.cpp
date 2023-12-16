#include "stdafx.h"

#include "SpeedUI.h"

#include "Text.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
SpeedUI::SpeedUI()
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}}}
{
    Text::Params txtParams;
    txtParams.parentHorizontalAnchor = Alignment::END;
    txtParams.parentVerticalAnchor = Alignment::START;
    txtParams.localHorizontalAnchor = Alignment::START;
    txtParams.localVerticalAnchor = Alignment::START;
    txtParams.transform.position.y = 50;
    txtParams.transform.position.x = -250;
    txtParams.transform.scale = Vector2{0.7f, 0.7f};
    txtParams.text = "SPEED";
    txtParams.font = WindowsEngine::GetDefaultFont();
    speedText = dynamic_cast<Text*>(AddChild(CreatePtr<Text>(txtParams)));
}

std::string SpeedUI::ConvertSpeed(const float speed) const
{
    float fakespeed = speed * 100.0f;
    int speedInt = static_cast<int>(fakespeed);
    int speedIntCenti = static_cast<int>((fakespeed - speedInt) * 100);
    return std::format("{:02d}.{:02d} mm/d", speedInt, speedIntCenti);
}

void SpeedUI::Update(const float dt, const float speed)
{
    UIElement::Update(dt);
    elapsedTime += dt;
    currentSpeed = std::lerp(currentSpeed, speed, dt * 5);
    speedText->SetText(ConvertSpeed(currentSpeed));
}
}
