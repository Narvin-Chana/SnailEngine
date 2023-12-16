#include "stdafx.h"
#include "WinScreen.h"

#include <random>

#include "Rendering/Texture2D.h"

#include "Button.h"
#include "Sprite.h"
#include "Text.h"
#include "Core/WindowsEngine.h"

namespace Snail
{
WinScreen::WinScreen()
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}}}
    , buttonTextureBG{std::make_unique<Texture2D>("Resources/button_wood_dark.png")}
    , buttonTextureHover{std::make_unique<Texture2D>("Resources/button_wood.png")}
    , titleTex{std::make_unique<Texture2D>("Resources/wood_sign.png")}
    , background{std::make_unique<Texture2D>(1, 1, Color{0, 0, 0, 128})}
{
    

    Sprite::Params bgParams;
    bgParams.sizeType = SizeType::TYPE_PERCENTAGE;
    bgParams.size = Vector2{1, 1};
    bgParams.texture = background.get();
    bgParams.parentHorizontalAnchor = Alignment::MIDDLE;
    bgParams.parentVerticalAnchor = Alignment::MIDDLE;
    bgParams.localHorizontalAnchor = Alignment::MIDDLE;
    bgParams.localVerticalAnchor = Alignment::MIDDLE;
    AddChild(CreatePtr<Sprite>(bgParams));

    Sprite::Params titleParams;
    titleParams.sizeType = SizeType::TYPE_ABSOLUTE;
    titleParams.texture = titleTex.get();
    titleParams.size = Vector2{325, 250};
    titleParams.transform.scale.y = -1;
    titleParams.parentHorizontalAnchor = Alignment::MIDDLE;
    titleParams.parentVerticalAnchor = Alignment::END;
    titleParams.localHorizontalAnchor = Alignment::MIDDLE;
    titleParams.localVerticalAnchor = Alignment::START;
    AddChild(CreatePtr<Sprite>(titleParams));

    Text::Params txtParams;
    txtParams.parentHorizontalAnchor = Alignment::MIDDLE;
    txtParams.parentVerticalAnchor = Alignment::END;
    txtParams.localHorizontalAnchor = Alignment::MIDDLE;
    txtParams.localVerticalAnchor = Alignment::END;
    txtParams.transform.position.y = -120;
    txtParams.transform.scale = Vector2{1, 1};
    txtParams.text = "WIN";
    txtParams.font = WindowsEngine::GetDefaultFont();
    AddChild(CreatePtr<Text>(txtParams));

    Button::Params btnParams;
    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.size = Vector2{150, 75};
    btnParams.transform.position = Vector2{0, 0};
    btnParams.text = "Restart";
    btnParams.onClick = [this]
    {
        static auto* scene = WindowsEngine::GetScene();
        scene->Reset();
    };
    btnParams.backgroundTexture = buttonTextureBG.get();
    btnParams.hoveredTexture = buttonTextureHover.get();
    AddChild(CreatePtr<Button>(btnParams));

    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{};
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.transform.position = Vector2{0, -120};
    btnParams.size = Vector2{150, 75};
    btnParams.text = "Quit";
    btnParams.onClick = [this]
    {
        static auto& engine = WindowsEngine::GetInstance();
        engine.Exit();
    };
    btnParams.backgroundTexture = buttonTextureBG.get();
    btnParams.hoveredTexture = buttonTextureHover.get();
    AddChild(CreatePtr<Button>(btnParams));

    txtParams.parentHorizontalAnchor = Alignment::START;
    txtParams.parentVerticalAnchor = Alignment::END;
    txtParams.localHorizontalAnchor = Alignment::START;
    txtParams.localVerticalAnchor = Alignment::START;
    txtParams.transform.position.y = -200.0f;
    txtParams.transform.position.x = 50.0f;
    txtParams.transform.scale = Vector2{0.7f, 0.7f};
    txtParams.text = "TIMER";
    txtParams.font = WindowsEngine::GetDefaultFont();
    timeText = dynamic_cast<Text*>(AddChild(CreatePtr<Text>(txtParams)));
}

std::string WinScreen::ConvertTime(const float time) const
{
    int minutes = static_cast<int>(time) / 60;
    int remainingSeconds = static_cast<int>(time) % 60;
    float milliseconds = (time - static_cast<int>(time)) * 1000;

    return std::format("{:02d}:{:02d}:{:03d}", minutes, remainingSeconds, static_cast<int>(milliseconds));
}

void WinScreen::SetTimesText(float time, std::vector<float> lapTimes)
{
    timeText->SetText("Race : " + ConvertTime(time));
    //get index of the best lap
    const int bestLapIndex = static_cast<int>(std::distance(lapTimes.begin(), std::ranges::min_element(lapTimes)));

    for (int i = 0; i < lapTimes.size(); ++i)
    {
        if (i == bestLapIndex)
            lapsTexts[i]->SetSize({ 1.1f,1.1f });
        else
            lapsTexts[i]->SetSize({0.9f,0.9f});
        lapsTexts[i]->SetText("Lap " + std::to_string(i + 1) + " : " + ConvertTime(lapTimes[i]));
    }
}

void WinScreen::SetLapCount(int lapCount)
{
    for (const auto txt : lapsTexts)
        timeText->RemoveChild(txt);

    lapsTexts.clear();
    for (int i = 0; i < lapCount; ++i)
    {
        Text::Params ResultParams;
        ResultParams.parentHorizontalAnchor = Alignment::START;
        ResultParams.parentVerticalAnchor = Alignment::START;
        ResultParams.localHorizontalAnchor = Alignment::START;
        ResultParams.localVerticalAnchor = Alignment::START;
        ResultParams.transform.position.y = -static_cast<float>(i + 1) * 50.0f;
        ResultParams.transform.position.x = 10;
        ResultParams.transform.scale = Vector2{ 0.9f, 0.9f };
        ResultParams.text = "TIMER";
        ResultParams.font = WindowsEngine::GetDefaultFont();
        lapsTexts.push_back(dynamic_cast<Text*>(timeText->AddChild(CreatePtr<Text>(ResultParams))));
    }
}

}
