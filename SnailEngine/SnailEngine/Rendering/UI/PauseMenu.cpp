#include "stdafx.h"
#include "PauseMenu.h"

#include <random>

#include "Rendering/Texture2D.h"

#include "Button.h"
#include "Sprite.h"
#include "Text.h"
#include "Core/WindowsEngine.h"

#include <chrono>

namespace Snail
{
PauseMenu::PauseMenu()
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}, .zOrder = 10}}
{
    SetActive(false);
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();
    buttonTextureBG = tm.GetTexture2D("Resources/button_wood_dark.png", true);
    buttonTextureHover = tm.GetTexture2D("Resources/button_wood.png", true);
    titleTex = tm.GetTexture2D("Resources/wood_sign.png", true);
    banana = tm.GetTexture2D("Resources/DK.png", true);
    background = tm.GetAsset<Texture2D>("MenuBackdrop");
    if (!background)
    {
        auto transparent = std::make_unique<Texture2D>(1, 1, Color{0, 0, 0, 128});
        background = tm.SaveAsset("MenuBackdrop", std::move(transparent), true);
    }

    Sprite::Params bgParams;
    bgParams.sizeType = SizeType::TYPE_PERCENTAGE;
    bgParams.size = Vector2{1, 1};
    bgParams.texture = background;
    bgParams.parentHorizontalAnchor = Alignment::MIDDLE;
    bgParams.parentVerticalAnchor = Alignment::MIDDLE;
    bgParams.localHorizontalAnchor = Alignment::MIDDLE;
    bgParams.localVerticalAnchor = Alignment::MIDDLE;
    AddChild(CreatePtr<Sprite>(bgParams));

    /*std::mt19937 mt((int)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> d{-800, 800};
    for (int i = 0; i < 100; ++i)
    {
        Sprite::Params params;
        params.size = Vector2{100, 100};
        params.texture = banana;
        params.parentHorizontalAnchor = Alignment::MIDDLE;
        params.parentVerticalAnchor = Alignment::MIDDLE;
        params.localHorizontalAnchor = Alignment::MIDDLE;
        params.localVerticalAnchor = Alignment::MIDDLE;
        params.transform.rotation = d(mt);
        params.transform.position = Vector2{d(mt), d(mt)};
        AddChild(CreatePtr<Sprite>(params));
    }*/

    Sprite::Params titleParams;
    titleParams.sizeType = SizeType::TYPE_ABSOLUTE;
    titleParams.texture = titleTex;
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
    txtParams.text = "PAUSE";
    txtParams.font = WindowsEngine::GetDefaultFont();
    AddChild(CreatePtr<Text>(txtParams));

    Button::Params btnParams;
    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.size = Vector2{150, 75};
    btnParams.transform.position = Vector2{0, -120};
    btnParams.text = "Restart";
    btnParams.onClick = [&]{
        static auto* scene = WindowsEngine::GetScene();
        scene->Reset();
    };
    btnParams.backgroundTexture = buttonTextureBG;
    btnParams.hoveredTexture = buttonTextureHover;
    AddChild(CreatePtr<Button>(btnParams));

    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{};
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.size = Vector2{150, 75};
    btnParams.text = "Resume";
    btnParams.onClick = [&]{
        static auto& gameManager = WindowsEngine::GetModule<GameManager>();
        gameManager.SetPaused(false);
    };
    btnParams.backgroundTexture = buttonTextureBG;
    btnParams.hoveredTexture = buttonTextureHover;
    AddChild(CreatePtr<Button>(btnParams));

    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{};
    btnParams.transform.scale = Vector2{ 1.5, 1.5 };
    btnParams.transform.position = Vector2{ 0, -240 };
    btnParams.size = Vector2{ 150, 75 };
    btnParams.text = "Main Menu";
    btnParams.onClick = [] {
        static auto& engine = WindowsEngine::GetInstance();
        if (Camera* camera = WindowsEngine::GetCamera())
            camera->RemoveOverlay(GameManager::PAUSE_MENU_UI_NAME);
        engine.GetScene()->LoadScene("Resources/Scenes/MainMenu.json");
        };
    AddChild(CreatePtr<Button>(btnParams));

    btnParams.parentHorizontalAnchor = Alignment::MIDDLE;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{};
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.transform.position = Vector2{0, -360};
    btnParams.size = Vector2{150, 75};
    btnParams.text = "Quit";
    btnParams.onClick = []{
        static auto& engine = WindowsEngine::GetInstance();
        engine.Exit();
    };
    btnParams.backgroundTexture = buttonTextureBG;
    btnParams.hoveredTexture = buttonTextureHover;
    AddChild(CreatePtr<Button>(btnParams));
}

void PauseMenu::Update(float x)
{
    UIElement::Update(x);
}
}
