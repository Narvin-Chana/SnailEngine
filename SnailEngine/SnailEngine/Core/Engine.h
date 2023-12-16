#pragma once
#include <memory>
#include <vector>

#include "Rendering/Device.h"
#include "Rendering/UI/Font.h"

#include "Assets/ModuleManager.h"
#include "Assets/MeshManager.h"
#include "Assets/TextureManager.h"

#include "Core/Camera/CameraManager.h"
#include "Core/Scene.h"
#include "GamePlay/GameManager.h"
#include "Physics/PhysicsModule.h"
#include "Util/Singleton.h"

#include "Rendering/UI/PauseMenu.h"

#include "RendererModule.h"

namespace Snail
{
struct InputEventHandler;

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
class Engine : public Singleton<T>
{
public:
    virtual void Run();
    virtual int Init();
    virtual void Update();

    TDeviceType* GetRenderDevice();

    template<class U>
    static U& GetModule()
    {
        return Engine::GetInstance().modules.template Get<U>();
    }

    static Scene* GetScene() { return T::GetInstance().scene.get(); }
    static Font* GetDefaultFont() { return T::GetInstance().defaultFont.get(); }

    static Camera* GetCamera()
    {
        static CameraManager& camManager = GetModule<CameraManager>();
        return camManager.GetCurrentCamera();
    }

    void SetPaused(bool setPaused);
    bool IsPaused() const;
    float GetDeltaTime() const;

    void Exit();

protected:
    Engine();
    ~Engine() override;

    // Spécifiques - Doivent être implémentés
    virtual int InitSpecific() = 0;
    virtual TDeviceType* CreateDeviceSpecific(Device::DisplayMode mode) = 0;

    virtual void UpdateSpecific() = 0;
    virtual int64_t GetTimeSpecific() const = 0;
    virtual double GetTimeIntervalsInSec(int64_t start, int64_t stop) const = 0;

    virtual void ResizeSpecific() = 0;
    virtual void ResetClock() = 0;

    virtual void Cleanup();
    virtual void Resize(long width, long height);
    virtual void ToggleFullscreen();

    bool isInitialized = false;
    int64_t prevTime = 0;
    int64_t nextTime = 0;
    std::unique_ptr<TDeviceType> renderDevice;
    std::vector<InputEventHandler*> eventHandlers;

public:

    bool isMainMenuLoaded = false;

protected: 

    std::unique_ptr<Scene> scene;
    std::unique_ptr<Font> defaultFont;

    std::unique_ptr<LoadingScreen> loadingScreen;

    // Modules
    ModuleManager<
        TextureManager,
        MeshManager,
        RendererModule,
        CameraManager,
        PhysicsModule,
        GameManager,
        DirectX::AudioEngine
    > modules;

    Device::DisplayMode currentMode{};
    bool isPaused = false;
    bool shouldExit = false;
    float frameDelta = 0;
};

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::Run()
{
    while (!shouldExit)
    {
        UpdateSpecific();
        Update();
    }
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
int Engine<T, TDeviceType>::Init()
{
    logger.SetLogFile("SnailEngine.log");

    // Propre à la plateforme
    InitSpecific();
    LOG("Specific init done.");

    currentMode = Device::DisplayMode::WINDOWED;

    renderDevice.reset(CreateDeviceSpecific(currentMode));
    LOG("Graphics device created");

    LOG("Loading modules...");

    // Initialise PhysX
    modules.RegisterModule<PhysicsModule>();
    LOG("Physics module initialized");

    // DirectX Tookit Audio Engine
    DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
    eflags |= DirectX::AudioEngine_Debug;
#endif
    modules.RegisterModule<DirectX::AudioEngine>(eflags);
    LOG("Audio module initialised");

    // Load asset manager modules once render device exists
    modules.RegisterModule<TextureManager>();
    LOG("Texture manager initialised");

    modules.RegisterModule<MeshManager>();
    LOG("Mesh manager initialised");

    defaultFont = std::make_unique<Font>();
    loadingScreen = std::make_unique<LoadingScreen>();

    modules.RegisterModule<CameraManager>();
    LOG("Camera manager initialised");

    modules.RegisterModule<GameManager>();
    LOG("Game manager initialised");

    scene = std::make_unique<Scene>();

    modules.RegisterModule<RendererModule>();
    LOG("Modules loaded");

    isInitialized = true;

    prevTime = nextTime = GetTimeSpecific();
    
    LOG("Engine init done");
    
    return 0;
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::Update()
{
    static InputModule& inputs = InputModule::GetInstance();

    inputs.Controller.DiscoverControllers();
    
    static auto& cameraManager = modules.Get<CameraManager>();
    static auto& physicsModule = modules.Get<PhysicsModule>();
    static auto& audioModule = modules.Get<DirectX::AudioEngine>();
    static auto& rendererModule = modules.Get<RendererModule>();
    static auto& gameManager = modules.Get<GameManager>();

    // Get elapsed time since previous frame
    const int64_t currentTime = GetTimeSpecific();
    if (const double dt = GetTimeIntervalsInSec(prevTime, currentTime))
    {
        frameDelta = static_cast<float>(dt);
        // Prepare next image
        renderDevice->Present();
        // On rend l'image sur la surface de travail
        // (tampon d'arrière plan)
        inputs.PreUpdate();

        if (!audioModule.Update())
        {
            if (audioModule.IsCriticalError())
            {
                LOG(Logger::FATAL, "Audio engine was unable to be updated...");
            }
        }
        
        if (scene->IsLoading())
        {
            SetPaused(false);
            loadingScreen->Update(frameDelta);
            std::lock_guard lock{DeviceMutex};
            loadingScreen->Draw();
        }
        else
        {
            if (!isPaused)
            {
                physicsModule.Update(frameDelta);
                scene->Update(frameDelta);
                rendererModule.Update(frameDelta);
                if (Camera* controlledCamera = cameraManager.GetControlledCamera(); controlledCamera)
                {
                    controlledCamera->UpdateControls(frameDelta);
                }
                if (Camera* viewCamera = cameraManager.GetCurrentCamera(); viewCamera) { viewCamera->Update(frameDelta); }
            }
            
            gameManager.Update(frameDelta);
            rendererModule.Render(scene.get());
        }

        scene->CleanupRemoveEntity();
        inputs.PostUpdate();

        // Dont roll back time if it has been reset
        if (prevTime <= currentTime)
            prevTime = currentTime;
    }
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
TDeviceType* Engine<T, TDeviceType>::GetRenderDevice() { return renderDevice.get(); }

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::SetPaused(const bool setPaused)
{
    isPaused = setPaused;
    if (!isPaused)
    {
        ResetClock();
    }
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
bool Engine<T, TDeviceType>::IsPaused() const
{
    return isPaused;
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
float Engine<T, TDeviceType>::GetDeltaTime() const
{
    return frameDelta;
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::Exit()
{
    shouldExit = true;
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
Engine<T, TDeviceType>::Engine() = default;

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
Engine<T, TDeviceType>::~Engine() { Engine::Cleanup(); }

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::Cleanup()
{
    if (scene)
        scene->Cleanup();

    static auto& audioModule = modules.Get<DirectX::AudioEngine>();
    audioModule.Suspend();

    renderDevice.reset();
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::Resize(long width, long height)
{
    if (!isInitialized)
        return;

    renderDevice->SetResolution(width, height);

    if (GetCamera() != nullptr)
        GetCamera()->Resize(width, height);

    ResizeSpecific();

    GetModule<RendererModule>().Resize(width, height);
}

template <class T, class TDeviceType> requires std::is_base_of_v<Device, TDeviceType>
void Engine<T, TDeviceType>::ToggleFullscreen()
{
    std::lock_guard lock{DeviceMutex};
    currentMode = currentMode == Device::DisplayMode::WINDOWED ? Device::DisplayMode::FULL_SCREEN : Device::DisplayMode::WINDOWED;
    renderDevice->SetDisplayMode(currentMode);

    auto [w, h] = renderDevice->GetResolutionSize();
    GetCamera()->Resize(w, h);

    ResizeSpecific();
}
} // namespace Snail
