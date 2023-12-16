#include "stdafx.h"

#include "LapUI.h"

#include "Text.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
    LapUI::LapUI(int maxlap)
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}}}
{
    maxLaps = maxlap;

    Text::Params txtParams;
    txtParams.parentHorizontalAnchor = Alignment::END;
    txtParams.parentVerticalAnchor = Alignment::START;
    txtParams.localHorizontalAnchor = Alignment::START;
    txtParams.localVerticalAnchor = Alignment::START;
    txtParams.transform.position.y = 100;
    txtParams.transform.position.x = -200;
    txtParams.transform.scale = Vector2{0.7f, 0.7f};
    txtParams.text = "LAP";
    txtParams.font = WindowsEngine::GetDefaultFont();
    lapText = dynamic_cast<Text*>(AddChild(CreatePtr<Text>(txtParams)));
}

void LapUI::Update(const float dt, const int lap)
{
    UIElement::Update(dt);
    lapText->SetText("Lap : " + std::to_string(lap+1) + "/" + std::to_string(maxLaps));
}
void LapUI::SetLapCount(int lapCount)
{
    maxLaps = lapCount;
}
}
