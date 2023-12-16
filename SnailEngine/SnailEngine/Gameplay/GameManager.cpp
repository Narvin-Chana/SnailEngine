#include "stdafx.h"

#include "GameManager.h"
#include "Core/WindowsEngine.h"
#include "Entities/vehicle.h"
#include "Rendering/Effects/PostProcessing/ChromaticAberrationEffect.h"

namespace Snail
{
void GameManager::Reset()
{
    lap = 0;
    checkpointIndex = 0;
    elapsedTime = 0;
    countdownValue = COUNTDOWN_SECONDS;

    lapTimes.clear();
    lapTimes.resize(numberOfLaps);
    std::ranges::fill(lapTimes, 0.0f);

    HideUI(winScreen);
    HideUI(pauseMenu);
    HideGeneralHUD();

    HideUI(transition);
    transition.anim.Reset();

    HideUI(countdown);
    countdown.anim.Reset();

    engineInstance->Stop(true);

    WindowsEngine::GetInstance().SetPaused(false);
}

void GameManager::Cleanup()
{
    Reset();

    checkpointCount = 0;
    vehicle = nullptr;
    door = nullptr;
}

void GameManager::Restart()
{
    Reset();
    Start();
}

void GameManager::Start()
{
    ShowUI(transition);
    engineInstance->SetVolume(SFX_VOLUME / 2.0f);
    if(!WindowsEngine::GetInstance().isMainMenuLoaded)
        engineInstance->Play(true);
    ShowGeneralHUD();
    state = TRANSITION;
}

GameManager::GameManager()
{
    static DirectX::AudioEngine& audioModule = WindowsEngine::GetModule<DirectX::AudioEngine>();
    finishedLapSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/lapComplete.wav");
    winSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/win.wav");
    boostPickupSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/boost_pickup.wav");
    keyPickupSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/keyJingle.wav");
    useBoostSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/useBoost.wav");
    countdownSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/countdown.wav");
    engineSFX = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/engine.wav");
    engineInstance = engineSFX->CreateInstance();
    backgroundMusic1 = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/BackgroundMusic3.wav");
    backgroundMusic2 = std::make_unique<DirectX::SoundEffect>(&audioModule, L"Resources/Audio/garden_ambiance.wav");
    backgroundMusicInstance = backgroundMusic2->CreateInstance();
    backgroundMusicInstance->SetVolume(SFX_VOLUME);
    backgroundMusicInstance->Play(true); 
    backgroundMusicInstance = backgroundMusic1->CreateInstance();
    backgroundMusicInstance->SetVolume(SFX_VOLUME);
    backgroundMusicInstance->Play(true);

    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();

    Sprite::Params mappingParams;
    mappingParams.texture = tm.GetTexture2D("Resources/controller_controls.png", true);
    mappingParams.parentHorizontalAnchor = UIElement::Alignment::START;
    mappingParams.parentVerticalAnchor = UIElement::Alignment::START;
    mappingParams.localHorizontalAnchor = UIElement::Alignment::START;
    mappingParams.localVerticalAnchor = UIElement::Alignment::START;
    mappingParams.size = mappingParams.texture->GetDimensions() * 1.5f;
    mappingParams.name = CONTROLS_UI_NAME;
    controllerMapping = Sprite::Create<Sprite>(mappingParams);

    Sprite::Params boostUIParams;
    boostUIParams.texture = tm.GetTexture2D("Resources/energyDrinkIcone.png", true);
    boostUIParams.parentHorizontalAnchor = UIElement::Alignment::END;
    boostUIParams.transform.position.y = 150;
    boostUIParams.transform.position.x = -150;
    boostUIParams.size = boostUIParams.texture->GetDimensions() * 0.25f;
    boostUIParams.name = BOOST_UI_NAME;
    boostIcon = Sprite::Create<Sprite>(boostUIParams);

    winScreen.name = WIN_SCREEN_UI_NAME;
    transition.name = TRANSITION_UI_NAME;
    timeUI.name = TIME_UI_NAME;
    lapUI.name = LAP_UI_NAME;
    timeLapUI.name = LAPTIME_UI_NAME;
    speedUI.name = SPEED_UI_NAME;
    pauseMenu.name = PAUSE_MENU_UI_NAME;
    pauseMenu.zOrder = 30;

    CountdownText::Params par;
    par.font = WindowsEngine::GetDefaultFont();
    par.parentHorizontalAnchor = UIElement::Alignment::MIDDLE;
    par.parentVerticalAnchor = UIElement::Alignment::MIDDLE;
    par.localHorizontalAnchor = UIElement::Alignment::MIDDLE;
    par.localVerticalAnchor = UIElement::Alignment::MIDDLE;
    countdown = UIElement::Create<CountdownText>(par);

    Reset();
}

void GameManager::Update(const float dt)
{
    switch (state)
    {
    case TRANSITION:
        UpdateTransition(dt);
        break;
    case START:
        UpdateStart(dt);
        break;
    case PLAY:
        UpdatePlay(dt);
        break;
    case PAUSE:
        UpdatePause(dt);
        break;
    case WIN:
        UpdateWin(dt);
        break;
    default:;
    }

    UpdateHUDs(dt);

    static WindowsEngine& engine = WindowsEngine::GetInstance();
    static InputModule& input = InputModule::GetInstance();

    const auto _state = input.Keyboard.GetState();
    const Controller* controller = input.Controller.GetFirstActiveController();
    if (!IsWin() && (_state.IsKeyPressed(input.Keyboard.ESCAPE) || (controller && controller->IsPressed(Controller::Buttons::START))))
    {
        SetPaused(!engine.IsPaused());
    }

    UpdateCarSFXPitch();
    UpdateBoostVFX(dt);
}

std::vector<float> GameManager::GetLapTimes() const
{
    return lapTimes;
}

int GameManager::GetNumberOfLaps() const
{
    return numberOfLaps;
}

void GameManager::ShowGeneralHUD()
{
    if (WindowsEngine::GetInstance().isMainMenuLoaded) return;
    ShowUI(timeUI);
    ShowUI(speedUI);
    ShowUI(lapUI);
    ShowUI(controllerMapping);
    ShowUI(timeLapUI);
}

void GameManager::UpdateHUDs(float dt)
{
    if (WindowsEngine::GetInstance().isMainMenuLoaded) return;
    speedUI.Update(dt, lastCarSpeed);
    timeUI.Update(dt, elapsedTime);
    timeLapUI.Update(dt, lapTimes[std::min(lap, numberOfLaps - 1)]);
    controllerMapping.Update(dt);
    boostIcon.Update(dt);
    lapUI.Update(dt, lap);
}

void GameManager::UpdateCarSFXPitch()
{
    engineInstance->SetPitch(std::min(lastCarSpeed / 25.0f, 1.0f));
}
void GameManager::UpdateBoostVFX(float dt)
{
    static auto& renderer = WindowsEngine::GetModule<RendererModule>();
    static auto& cm = WindowsEngine::GetModule<CameraManager>();

    if (renderer.chromaticAberrationEffect->IsActive())
    {
        auto data = renderer.chromaticAberrationEffect->GetData();
        if (data.intensity > 0)
        {
            data.intensity -= dt * 15;
            renderer.chromaticAberrationEffect->SetData(data);
            auto fov = cm.GetFov();
            fov = std::max(fov - dt * 15.0f, 60.f);
            cm.SetFov(fov);
        }
        else
        {
            renderer.chromaticAberrationEffect->SetActive(false);
            WindowsEngine::GetModule<CameraManager>().SetFov(60);
        }
    }
}

void GameManager::IncrementLap()
{
    checkpointIndex = 0;
    lap++;
    if (lap == numberOfLaps)
        Win();
    else
        finishedLapSFX->Play();
}

void GameManager::HideGeneralHUD()
{
    HideUI(timeUI);
    HideUI(speedUI);
    HideUI(lapUI);
    HideUI(timeLapUI);
    HideUI(controllerMapping);
}

void GameManager::UpdateTransition(float dt)
{
    transition.Update(dt);

    if (transition.anim.IsDone() && !WindowsEngine::GetInstance().isMainMenuLoaded)
    {
        HideUI(transition);
        ShowUI(countdown);
        countdown.anim.Start();
        countdown.SetText(std::to_string(countdownValue));
        countdownSFX->Play(SFX_VOLUME, 0.0f, 0.0f);
        state = START;
    }
}
void GameManager::UpdateStart(float dt)
{
    countdown.Update(dt);
    if (countdown.anim.IsDone())
    {
        if (countdownValue > 1)
        {
            countdownValue--;
            countdown.SetText(std::to_string(countdownValue));
            countdown.anim.Reset();
        }
        else
        {
            countdown.SetText("GO!");
            countdown.anim.Reset();
            state = PLAY;
        }
    }
}

void GameManager::UpdatePlay(float dt)
{
    countdown.Update(dt);
    if (countdown.anim.IsDone())
        HideUI(countdown);

    elapsedTime += dt;
    lapTimes[lap] += dt;

    if (vehicle)
    {
        if (const PhysicsVehicle* physicsVehicle = vehicle->GetPhysicsVehicle())
        {
            lastCarSpeed = physicsVehicle->GetSpeed();
        }
    }
}

void GameManager::UpdatePause(float dt)
{
    if (WindowsEngine::GetInstance().isMainMenuLoaded) return;
    pauseMenu.Update(dt);
}

void GameManager::UpdateWin(float dt)
{
    winScreen.Update(dt);
}

void GameManager::Win()
{
    if (state == WIN)
        return;

    engineInstance->Stop(true);
    winSFX->Play(SFX_VOLUME, 0.0f, 0.0f);

    HideGeneralHUD();
    HideUI(transition);
    HideUI(pauseMenu);

    WindowsEngine::GetInstance().SetPaused(true);

    winScreen.SetTimesText(elapsedTime, { lapTimes.begin(), lapTimes.end() });
    ShowUI(winScreen);

    state = WIN;
}

bool GameManager::IsWin() const
{
    return state == WIN;
}

bool GameManager::IsPaused() const
{
    return state == PAUSE;
}

bool GameManager::IsPlay() const
{
    return state == PLAY;
}

void GameManager::SetPaused(bool paused)
{
    static WindowsEngine& engine = WindowsEngine::GetInstance();
    if(engine.isMainMenuLoaded) return;
    engine.SetPaused(paused);

    if (paused)
    {
        ShowUI(pauseMenu);
        static InputModule& input = InputModule::GetInstance();
        // Enable mouse just incase it is not accessible
        input.Mouse.SetMode(Mouse::MODE_ABSOLUTE);
        input.Mouse.SetVisible(true);
        input.Mouse.SetLock(false);

        prevState = state;
        engineInstance->Stop(true);
        state = PAUSE;
    }
    else
    {
        HideUI(pauseMenu);
        engineInstance->Play(true);
        state = prevState;
    }
}
void GameManager::SetNumberOfLaps(int laps)
{
    numberOfLaps = laps;
    lapTimes.clear();
    lapTimes.resize(numberOfLaps);
    winScreen.SetLapCount(laps);
    lapUI.SetLapCount(laps);
    std::ranges::fill(lapTimes, 0.0f);
}

int GameManager::AddCheckpoint()
{
    return ++checkpointCount;
}

bool GameManager::TriggerCheckpoint(const int checkpoint)
{
    // If the checkpoint is the next one in the sequence
    if (checkpoint == checkpointIndex + 1)
    {
        // Set the checkpoint index to the new checkpoint
        checkpointIndex = checkpoint;
        return true;
    }
    // If the checkpoint is start and the current checkpoint is the last one
    if (checkpoint == 0 && checkpointIndex == checkpointCount - 1 && state != WIN)
    {
        // Set the checkpoint index to start
        IncrementLap();
        return true;
    }

    return false;
}
bool GameManager::HasBoost() const
{
    return vehicle->HasBoost();
}

void GameManager::CollectKey()
{
    if (!door)
        return;

    keyPickupSFX->Play(SFX_VOLUME, 0.0f, 0.0f);

    WindowsEngine::GetScene()->RemoveEntity(door);
    door = nullptr;
}

void GameManager::CollectBoost()
{
    if (vehicle)
        vehicle->CollectBoost();

    boostPickupSFX->Play(SFX_VOLUME, 0.0f, 0.0f);
    ShowUI(boostIcon);
}
void GameManager::UseBoost()
{
    useBoostSFX->Play(SFX_VOLUME, 0.0f, 0.0f);

    static auto& renderer = WindowsEngine::GetModule<RendererModule>();
    renderer.chromaticAberrationEffect->SetActive(true);
    renderer.screenShakeEffect.SetActive(true);
    renderer.vignetteEffect->SetActive(true);

    WindowsEngine::GetModule<CameraManager>().SetFov(90);

    ChromaticAberrationParameters chromaticData;
    chromaticData.intensity = 30;
    renderer.chromaticAberrationEffect->SetData(chromaticData);

    HideUI(boostIcon);
}

Vehicle* GameManager::GetVehicle() const
{
    return vehicle;
}

void GameManager::SetVehicle(Vehicle* _vehicle)
{
    vehicle = _vehicle;
}

void GameManager::SetDoor(Door* _door)
{
    door = _door;
}

void GameManager::ShowUI(UIElement& ui)
{
    if (const auto cam = WindowsEngine::GetCamera())
        cam->AddOverlay(ui.name, &ui);
    ui.SetActive(true);
}

void GameManager::HideUI(UIElement& ui)
{
    if (const auto cam = WindowsEngine::GetCamera())
        cam->RemoveOverlay(ui.name);
    ui.SetActive(false);
}

void GameManager::RenderImGui()
{
#ifdef _IMGUI_
    ImGui::Text(std::format("Max Laps: {}", numberOfLaps).c_str());
    ImGui::Text(std::format("Current Lap: {}", lap).c_str());
    ImGui::Separator();
    ImGui::Text(std::format("Checkpoint count: {}", checkpointCount).c_str());
    ImGui::Text(std::format("Current checkpoint id: {}", checkpointIndex).c_str());
    ImGui::Separator();

    static constexpr const char* names[] = {"TRANSITION", "START", "PLAY", "PAUSE", "WIN"};
    ImGui::Text("Current State: %s", names[static_cast<int>(state)]);

    if (ImGui::Button("Win"))
        Win();
    ImGui::SameLine();
    if (ImGui::Button("Restart"))
        Restart();
    ImGui::SameLine();
    if (ImGui::Button("Complete Lap"))
        IncrementLap();
    ImGui::SameLine();
    if (ImGui::Button("Get Boost"))
        CollectBoost();

    ImGui::Separator();
    ImGui::Text("Shortcut state: %s", door ? "CLOSED" : "OPEN");

    int lapCount = numberOfLaps;
    if (ImGui::InputInt("Lap Count", &lapCount))
        SetNumberOfLaps(std::max(lapCount, 0));
#endif
}
}
