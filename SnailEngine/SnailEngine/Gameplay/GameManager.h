#pragma once

#include "Core/Physics/PhysicsVehicle.h"
#include "Entities/vehicle.h"
#include "Entities/Door.h"
#include "Rendering/UI/CountdownText.h"
#include "Rendering/UI/SceneTransition.h"
#include "Rendering/UI/PauseMenu.h"
#include "Rendering/UI/TimeUI.h"
#include "Rendering/UI/SpeedUI.h"
#include "Rendering/UI/WinScreen.h"
#include "Rendering/UI/LapUI.h"

namespace Snail
{
class GameManager
{
public:
    static constexpr const char* BOOST_UI_NAME = "boostIcon";
    static constexpr const char* TIME_UI_NAME = "timeUI";
    static constexpr const char* LAP_UI_NAME = "lapUI";
    static constexpr const char* SPEED_UI_NAME = "speedUI";
    static constexpr const char* CONTROLS_UI_NAME = "controllerControls";
    static constexpr const char* TRANSITION_UI_NAME = "transition";
    static constexpr const char* WIN_SCREEN_UI_NAME = "winScreen";
    static constexpr const char* PAUSE_MENU_UI_NAME = "pauseMenu";
    static constexpr const char* COUNTDOWN_UI_NAME = "countdown";
    static constexpr const char* LAPTIME_UI_NAME = "lapTimeUI";
    static constexpr int COUNTDOWN_SECONDS = 3;
    static constexpr float SFX_VOLUME = 0.1f;

    //Scene change
    GameManager();
    void Reset();
    void Cleanup();
    void Restart();
    void Start();
    void Update(float dt);

    int AddCheckpoint();
    bool TriggerCheckpoint(int checkpoint);

    bool HasBoost() const;
    void CollectBoost();
    void UseBoost();

    void CollectKey();

    bool IsWin() const;
    bool IsPaused() const;
    bool IsPlay() const;

    void SetPaused(bool paused);

    std::vector<float> GetLapTimes() const;

    void SetVehicle(Vehicle*);
    Vehicle* GetVehicle() const;

    void SetNumberOfLaps(int laps);
    int GetNumberOfLaps() const;

    void SetDoor(Door*);

    void RenderImGui();

private:
    static void ShowUI(UIElement& ui);
    static void HideUI(UIElement& ui);

    void ShowGeneralHUD();
    void HideGeneralHUD();

    void UpdateTransition(float dt);
    void UpdateStart(float dt);
    void UpdatePlay(float dt);
    void UpdatePause(float dt);
    void UpdateWin(float dt);
    void UpdateHUDs(float dt);
    void UpdateCarSFXPitch();
    void UpdateBoostVFX(float dt);

    void IncrementLap();

    void Win();

    enum State
    {
        TRANSITION,
        START,
        PLAY,
        PAUSE,
        WIN
    } state = PAUSE;

    State prevState = PAUSE;

    // General game state
    int numberOfLaps = 3;
    int lap = 0;
    int checkpointCount = 0;
    int checkpointIndex = 0; // start = 0
    float elapsedTime = 0.0f;
    std::vector<float> lapTimes;

    // Sound-related
    std::unique_ptr<DirectX::SoundEffect>
        finishedLapSFX,
        boostPickupSFX,
        useBoostSFX,
        keyPickupSFX,
        winSFX,
        engineSFX,
        countdownSFX,
        backgroundMusic1,
        backgroundMusic2;

    std::unique_ptr<DirectX::SoundEffectInstance> backgroundMusicInstance, engineInstance;

    // UI
    WinScreen winScreen;
    PauseMenu pauseMenu;

    Sprite controllerMapping;
    Sprite boostIcon;
    SceneTransition transition;
    TimeUI timeUI;
    TimeUI timeLapUI{true};
    LapUI lapUI{3};
    SpeedUI speedUI;
    CountdownText countdown;

    int countdownValue = COUNTDOWN_SECONDS;
    Vehicle* vehicle = nullptr;
    Door* door = nullptr;
    float lastCarSpeed = 0.0f;
    
};
} // namespace Snail
