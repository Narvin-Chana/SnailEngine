#include "stdafx.h"
#include "MainMenu.h"

#include <random>

#include "Rendering/Texture2D.h"

#include "Button.h"
#include "Sprite.h"
#include "Text.h"
#include "Core/WindowsEngine.h"

#include <chrono>

namespace Snail
{
    MainMenu::MainMenu()
    : UIElement{{.sizeType = SizeType::TYPE_PERCENTAGE, .size = {1, 1}}}
{
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();
    buttonTextureBG = tm.GetTexture2D("Resources/button_wood_dark.png", true);
    buttonTextureHover = tm.GetTexture2D("Resources/button_wood.png", true);
    titleTex = tm.GetTexture2D("Resources/wood_sign.png", true);
    banana = tm.GetTexture2D("Resources/DK.png", true);
    background = tm.GetTexture2D("Resources/backgroundMenu.png", true);
    logo = tm.GetTexture2D("Resources/logo.png", true);

    Sprite::Params titleParams;
    titleParams.sizeType = SizeType::TYPE_ABSOLUTE;
    titleParams.texture = logo;
    titleParams.size = Vector2{450, 300};
    titleParams.transform.position = Vector2{ 0, -350 };
    titleParams.parentHorizontalAnchor = Alignment::MIDDLE;
    titleParams.parentVerticalAnchor = Alignment::END;
    titleParams.localHorizontalAnchor = Alignment::MIDDLE;
    titleParams.localVerticalAnchor = Alignment::START;
    AddChild(CreatePtr<Sprite>(titleParams));

    Button::Params btnParams;

    btnParams.parentHorizontalAnchor = Alignment::START;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{ 450, -60 };
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.size = Vector2{ 200, 75};
    btnParams.text = "Play";
    btnParams.onClick = [&]{
        static auto& engine = WindowsEngine::GetInstance();
        SetActive(false);
        if (Camera* camera = WindowsEngine::GetCamera())
            camera->RemoveOverlay(GameManager::PAUSE_MENU_UI_NAME);
        engine.GetScene()->LoadScene("Resources/Scenes/DefaultScene.json");
        engine.GetModule<GameManager>().Reset();
    };
    btnParams.backgroundTexture = buttonTextureBG;
    btnParams.hoveredTexture = buttonTextureHover;
    AddChild(CreatePtr<Button>(btnParams));

    btnParams.parentHorizontalAnchor = Alignment::START;
    btnParams.parentVerticalAnchor = Alignment::MIDDLE;
    btnParams.localHorizontalAnchor = Alignment::MIDDLE;
    btnParams.localVerticalAnchor = Alignment::MIDDLE;
    btnParams.transform.position = Vector2{};
    btnParams.transform.scale = Vector2{1.5, 1.5};
    btnParams.transform.position = Vector2{450, -200};
    btnParams.size = Vector2{200, 75};
    btnParams.text = "Quit";
    btnParams.onClick = []{
        static auto& engine = WindowsEngine::GetInstance();
        engine.Exit();
    };
    btnParams.backgroundTexture = buttonTextureBG;
    btnParams.hoveredTexture = buttonTextureHover;
    AddChild(CreatePtr<Button>(btnParams));
}

void MainMenu::Update(float x)
{
    UIElement::Update(x);
    WindowsEngine::GetScene()->SetVolumetricFactor(0);
}
}
